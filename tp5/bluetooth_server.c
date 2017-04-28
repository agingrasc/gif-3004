#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <vorbis/vorbisenc.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include "fifo.h"

#define BUFFER_FRAMES 64

int closing = 0;

long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

void gereSignal(int signo) {
    if (signo == SIGTERM || signo == SIGPIPE){
        closing = 1;
    }
}

int main (int argc, char *argv[]){
    int err;
    int eos=0;
    char buffer[BUFFER_FRAMES*2];

    if (argc != 2){
        printf("Must have 2 input arguments\n");
        exit(1);
    }

    signal(SIGTERM, gereSignal);
    signal(SIGPIPE, gereSignal);

    int out_fd = setr_fifo_writer("/tmp/bluetooth_out");
    int in_fd = setr_fifo_reader(argv[1]);
    FILE* pipe = fdopen(out_fd, "w");
    FILE* outfile = pipe;

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

    if ((err = vorbis_encode_init(&vi,1,44100, 64000, -1, -1))) {
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
            fwrite(og.header,1,og.header_len,outfile);
            fwrite(og.body,1,og.body_len,outfile);
        }

    }

    int offset = 0;
    while (!closing){

        int bytes_read; 
        if ((bytes_read = read(in_fd, buffer+offset, (BUFFER_FRAMES*2)-offset)) < 0){
            fprintf(stderr, "error reading pipe\n");
            exit(1);
        }

        printf("bytes_read: %i\n", bytes_read);

        int buffer_frames = (bytes_read >> 1);
        offset = bytes_read % 2;

        float **vorbis_buffer=vorbis_analysis_buffer(&vd,buffer_frames);
        for(int i = 0; i < (buffer_frames); i++){
            int16_t *sample = (int16_t*) &buffer[i*2];
            vorbis_buffer[0][i] = *sample / 32768.f;
        }

        if (offset == 1)
            buffer[0] = buffer[bytes_read-1];

        long long start_time = current_timestamp();
        vorbis_analysis_wrote(&vd,buffer_frames);
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
                  fwrite(og.header,1,og.header_len,outfile);
                  fwrite(og.body,1,og.body_len,outfile);

                  /* this could be set above, but for illustrative purposes, I do
                     it here (to show that vorbis does know where the stream ends) */

                  if(ogg_page_eos(&og))eos=1;
                }
            }
        }
        printf("time(ms): %lu\n", current_timestamp()-start_time);
    }

    vorbis_analysis_wrote(&vd,0);

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    exit(1);

}
