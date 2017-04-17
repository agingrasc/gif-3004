
#include <alsa/asoundlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "fifo.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#define BUFFER_SIZE 10240

int reajust_buffer(unsigned char *buffer, int data_consumed, int data_in_buffer){
    int data_copied = data_in_buffer-data_consumed;
    memmove(buffer, buffer+data_consumed, data_copied);
    return data_copied;
}

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

int main (int argc, char *argv[]){

	snd_pcm_t *playback_handle;

    audio_open("sysdefault:CARD=LX3000", &playback_handle);
    int reader = setr_fifo_reader("/tmp/bluetooth_out"); 

    unsigned char buffer[BUFFER_SIZE];
    int data_in_buffer; 
    for(int i = 0; i<BUFFER_SIZE;){
        int bytes_read = read(reader, buffer+i, BUFFER_SIZE-i);
        if (bytes_read > 0)
            i += bytes_read;
    }
    data_in_buffer = BUFFER_SIZE;

    int data_consumed, vorbis_error=0;

    stb_vorbis* decoder = stb_vorbis_open_pushdata(buffer, data_in_buffer, &data_consumed, &vorbis_error, NULL);

    if (decoder == NULL && vorbis_error == VORBIS_need_more_data){
        printf("NEED MOAR DATA\n");
    }

    data_in_buffer = reajust_buffer(buffer, data_consumed, data_in_buffer);

    while(1){
        int data_read;
        if ((data_read = read(reader, buffer+data_in_buffer, BUFFER_SIZE-data_in_buffer)) != -1){
            float **sample_data;
            int channels, samples;
            int err;
            data_in_buffer += data_read;
            data_consumed = stb_vorbis_decode_frame_pushdata(decoder, buffer, data_in_buffer, &channels, &sample_data, &samples);
            printf("data_in_buffer: %i\n", data_in_buffer);
            printf("data_consumed: %i\n", data_consumed);
            if (samples > 0){
                int16_t buf[samples];
                for(int i=0; i<samples; i++){
                    buf[i] = sample_data[0][i] * ((1<<(16-1))-1);
                }
                if ((err = snd_pcm_writei (playback_handle, buf, samples)) < 0) {
                    fprintf (stderr, "write to audio interface failed (%s)\n",
                         snd_strerror (err));
                    exit (1);
                }
            }
            data_in_buffer = reajust_buffer(buffer, data_consumed, data_in_buffer);
        }
    }

    close(reader);
}
