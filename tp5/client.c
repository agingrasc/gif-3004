
#include <alsa/asoundlib.h>
#include <signal.h>
#include <pthread.h>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#define BUFFER_SIZE (256 * 1024)
#define EXPECTED_HEADER_SIZE 10240
#define RECEPTION_BUFFER_SIZE 8192

int receive_mode = 1;
int playback_active = 1;
unsigned char* received_data;
int reader_idx = 0;
int writer_idx = 0;


void audio_open(char* device, snd_pcm_t **playback_handle){
	int err;
	snd_pcm_hw_params_t *hw_params;

	// Voyez la documentation de Alsa pour plus de détails
	// De manière générale, la carte de son est d'abord ouverte, puis
	// on configure plusieurs paramètres.
	if ((err = snd_pcm_open (playback_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
			 device,
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_any (*playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (*playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (*playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

    unsigned int rate = 44100;
	if ((err = snd_pcm_hw_params_set_rate_near (*playback_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (*playback_handle, hw_params, 1)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

    unsigned int min=1000, max=50000;
    int mindir=1, maxdir=-1;
    if ((err = snd_pcm_hw_params_set_buffer_time_minmax (*playback_handle, hw_params, &min, &mindir, &max, &maxdir)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

    unsigned int val;
    int dir;
    snd_pcm_hw_params_get_buffer_time(hw_params, &val, &dir);
    printf("buffer_time: %u, dir: %i\n", val, dir);
    snd_pcm_hw_params_get_buffer_time_min(hw_params, &val, &dir);
    printf("buffer_time (min): %u, dir: %i\n", val, dir);
    snd_pcm_hw_params_get_buffer_time_max(hw_params, &val, &dir);
    printf("buffer_time (max): %u, dir: %i\n", val, dir);

	if ((err = snd_pcm_hw_params (*playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (*playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
}


int get_availables_bytes() {
    int actual_writer_idx = writer_idx;
    if (actual_writer_idx < reader_idx) {
        int availables_bytes = BUFFER_SIZE - reader_idx;
        reader_idx = 0;
        return availables_bytes;
    }
    return actual_writer_idx - reader_idx;
}


int write_data(char* data, ssize_t size) {

    int remaining_bytes_before_roll_over = BUFFER_SIZE - writer_idx;
    if (remaining_bytes_before_roll_over < size) {
        memcpy(received_data, data, remaining_bytes_before_roll_over);
        writer_idx = 0;
        data += remaining_bytes_before_roll_over;
        size -= remaining_bytes_before_roll_over;
    }
    memcpy(received_data + writer_idx, data, (int) size);
    writer_idx += (int) size;
    return 0;
}


void* reception_thread(void* args) {
    char reception_data[RECEPTION_BUFFER_SIZE];
    int bluetooth_reader = *((int*) args);
    while (receive_mode) {
        ssize_t bytes_read = read(bluetooth_reader, reception_data, RECEPTION_BUFFER_SIZE);
        write_data(reception_data, bytes_read);
    }
}


int main (int argc, char *argv[]){

    if (argc < 3){
        printf("Need an audio device!\n");
        printf("Need a file path\n");
        exit(1);
    }

	snd_pcm_t *playback_handle;

    audio_open(argv[1], &playback_handle);
    int reader = open(argv[2], O_RDONLY);
    if (reader == -1) {
        printf("Open failed! %s", argv[2]);
        exit(1);
    }
    printf("Open sucessful\n");

    received_data = (unsigned char *) malloc(BUFFER_SIZE);
    if (received_data == NULL) {
        printf("Echec allocation du buffer.\n");
        exit(1);
    }

    pthread_t thread;
    pthread_create(&thread, NULL, reception_thread, &reader);
    while (get_availables_bytes() < EXPECTED_HEADER_SIZE) {
        sleep(0);
    }
    int data_in_buffer = get_availables_bytes();
    printf("Header ok, we have: %d\n", get_availables_bytes());

    int data_consumed, vorbis_error=0;

    stb_vorbis* decoder = stb_vorbis_open_pushdata(received_data, data_in_buffer, &data_consumed, &vorbis_error, NULL);
    reader_idx = (reader_idx + data_consumed) % BUFFER_SIZE;

    if (decoder == NULL && vorbis_error == VORBIS_need_more_data){
        printf("NEED MOAR DATA\n");
    }

    snd_pcm_sframes_t delay;
    while(playback_active){
        int availables_bytes = get_availables_bytes();
        if (availables_bytes <= 0) {
            sleep(0);
            continue;
        }
        data_in_buffer = availables_bytes;

        float **sample_data;
        int channels, samples;
        int err;
        data_consumed = stb_vorbis_decode_frame_pushdata(decoder, received_data+reader_idx, data_in_buffer, &channels, &sample_data, &samples);
        reader_idx = (reader_idx + data_consumed) % BUFFER_SIZE;

        #ifdef DEBUG
            printf("data_in_buffer: %i\n", data_in_buffer);
            printf("data_consumed: %i\n", data_consumed);
            if(snd_pcm_delay(playback_handle, &delay) == 0) {
                printf("delay: %f\n", delay / 44100.0f);
            }
        #endif

        if (samples > 0){
            printf("Playing some samples: %d\n", samples);
            int16_t buf[samples];

            for (int i = 0; i < samples; i++){
                buf[i] = sample_data[0][i] * ((1<<(16-1))-1);
            }

            if ((err = snd_pcm_writei (playback_handle, buf, samples)) < 0) {
                if (err == -EPIPE){
                    printf("pipe died\n"); 
                    snd_pcm_prepare(playback_handle);
                    snd_pcm_writei (playback_handle, buf, samples);
                }
                else{
                    fprintf (stderr, "write to audio interface failed (%s)\n",
                        snd_strerror (err));
                    exit (1);
                }
            }
        }
    }

    void* thread_ret;
    pthread_join(thread, thread_ret);
    close(reader);
}

void sighandler(int signum) {
    if (signum == SIGINT || signum == SIGKILL) {
        receive_mode = 0;
        playback_active = 0;
    }
}
