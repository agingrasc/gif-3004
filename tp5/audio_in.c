#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <vorbis/vorbisenc.h>
#include <alsa/asoundlib.h>
#include "fifo.h"

#define BUFFER_FRAMES 128

int closing = 0;

void gereSignal(int signo) {
    if (signo == SIGTERM || signo == SIGPIPE){
        closing = 1;
    }
}

int open_sound(char* device, snd_pcm_t **capture_handle, char** buffer){
    int err;
    unsigned int rate = 44100;				// Attention : la fréquence que vous choisirez ici devra vous suivre tout le long du traitement!
    snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;	// Même chose pour le format!

      // Tout comme pour la lecture, on doit d'abord ouvrir le périphérique
    // et enregistrer ses paramètres
    if ((err = snd_pcm_open (capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
                 device,
                 snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "audio interface opened\n");

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "hw_params allocated\n");

    if ((err = snd_pcm_hw_params_any (*capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "hw_params initialized\n");

    if ((err = snd_pcm_hw_params_set_access (*capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "hw_params access set\n");

    if ((err = snd_pcm_hw_params_set_format (*capture_handle, hw_params, format)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "hw_params format set\n");

    if ((err = snd_pcm_hw_params_set_rate_near (*capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "hw_params rate set\n");

    if ((err = snd_pcm_hw_params_set_channels (*capture_handle, hw_params, 1)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "hw_params channels setted\n");

    unsigned int min=1000, max=50000;
    int mindir=1, maxdir=-1;
    if ((err = snd_pcm_hw_params_set_buffer_time_minmax (*capture_handle, hw_params, &min, &mindir, &max, &maxdir)) < 0) {
		fprintf (stderr, "cannot set buffer time (%s)\n",
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

    if ((err = snd_pcm_hw_params (*capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "hw_params set\n");

    snd_pcm_hw_params_free (hw_params);

    fprintf(stderr, "hw_params freed\n");

    if ((err = snd_pcm_prepare (*capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    fprintf(stderr, "audio interface prepared\n");

    *buffer = (char*) malloc(BUFFER_FRAMES * snd_pcm_format_width(format) / 8 * 2);

    fprintf(stderr, "buffer allocated\n");

    return 0;
}

int close_sound(snd_pcm_t *capture_handle, char* buffer){
    free(buffer);

    fprintf(stderr, "buffer freed\n");

    snd_pcm_close (capture_handle);
    fprintf(stderr, "audio interface closed\n");

    return 0;
}

int main (int argc, char *argv[]){
    int err;
    int eos=0;
    char *buffer;
    snd_pcm_t *capture_handle;
    int buffer_frames = BUFFER_FRAMES;

    if (argc != 2){
        printf("Must have 1 input arguments\n");
        exit(1);
    }

    signal(SIGTERM, gereSignal);
    signal(SIGPIPE, gereSignal);

    int pipe_fd = setr_fifo_writer("/tmp/setr_audio_source");

    open_sound(argv[1], &capture_handle, &buffer);

    snd_pcm_sframes_t delay;

    while (!closing){
        // La fonction snd_pcm_readi est la clé, c'est elle qui permet d'acquérir le signal
        // de manière effective et de le transférer dans un buffer
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            fprintf (stderr, "read from audio interface failed (%s)\n",
                     err, snd_strerror (err));
            exit (1);
        }

        if(snd_pcm_delay(capture_handle, &delay) == 0)
            printf("delay: %f\n", delay/44100.0f); 

        if ((err = write(pipe_fd, buffer, buffer_frames*2)) == -1){
            fprintf(stderr, "write to buffer failed\n");
            exit(1);
        }
    }

    close_sound(capture_handle, buffer);

    exit(0);

}
