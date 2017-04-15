#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <vorbis/vorbisenc.h>
#include <alsa/asoundlib.h>

#define READ 1024
signed char readbuffer[READ*4+44]; /* out of the data segment, not the stack */ 

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

    *buffer = (char*) malloc(128 * snd_pcm_format_width(format) / 8 * 2);

    fprintf(stderr, "buffer allocated\n");
}

int close_sound(snd_pcm_t *capture_handle, char* buffer){
    free(buffer);

    fprintf(stderr, "buffer freed\n");

    snd_pcm_close (capture_handle);
    fprintf(stderr, "audio interface closed\n");
}

int main (int argc, char *argv[]){
    int err;
    int eos=0;
    char *buffer;
    snd_pcm_t *capture_handle;
    int buffer_frames = 128;

    open_sound(argv[1], &capture_handle, &buffer);

    ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
    ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */

    vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                          settings */
    vorbis_comment   vc; /* struct that stores all the user comments */

    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    vorbis_info_init(&vi);

    if ((err = vorbis_encode_init(&vi,1,44100, -1, 128000, -1))) {
        printf("vorbis_encode_init failed\n");
        exit(1);
    }

    vorbis_comment_init(&vc);

      /* set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&vd,&vi);
    vorbis_block_init(&vd,&vb);
    
    srand(time(NULL));
    ogg_stream_init(&os,rand());

    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);
        ogg_stream_packetin(&os,&header); /* automatically placed in its own
                                             page */
        ogg_stream_packetin(&os,&header_comm);
        ogg_stream_packetin(&os,&header_code);

        /* This ensures the actual
         * audio data will start on a new page, as per spec
         */
        while(!eos){
            int result=ogg_stream_flush(&os,&og);
            if(result==0)break;
            fwrite(og.header,1,og.header_len,stdout);
            fwrite(og.body,1,og.body_len,stdout);
        }

    }

    for (int i = 0; i < 5000; ++i) {
        // La fonction snd_pcm_readi est la clé, c'est elle qui permet d'acquérir le signal
        // de manière effective et de le transférer dans un buffer
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            fprintf (stderr, "read from audio interface failed (%s)\n",
                     err, snd_strerror (err));
            exit (1);
        }
        float **vorbis_buffer=vorbis_analysis_buffer(&vd,128);
        for(int i = 0; i < (128); i++){
            int16_t *sample = (int16_t*) &buffer[i*2];
            vorbis_buffer[0][i] = *sample / 32768.f;
        }
        vorbis_analysis_wrote(&vd,128);
        while(vorbis_analysis_blockout(&vd,&vb)==1){

            /* analysis, assume we want to use bitrate management */
            vorbis_analysis(&vb,NULL);
            vorbis_bitrate_addblock(&vb);

            while(vorbis_bitrate_flushpacket(&vd,&op)){

                /* weld the packet into the bitstream */
                ogg_stream_packetin(&os,&op);

                /* write out pages (if any) */
                while(!eos){
                  int result=ogg_stream_pageout(&os,&og);
                  if(result==0)break;
                  fwrite(og.header,1,og.header_len,stdout);
                  fwrite(og.body,1,og.body_len,stdout);

                  /* this could be set above, but for illustrative purposes, I do
                     it here (to show that vorbis does know where the stream ends) */

                  if(ogg_page_eos(&og))eos=1;
                }
            }
        }
        fprintf(stderr, "read %d done\n", i);
    }

    vorbis_analysis_wrote(&vd,0);

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    close_sound(capture_handle, buffer);

}
