
#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#define BUFFER_SIZE 1 

#define SAMPLE_RATE 44100

float alpha = 1;
size_t N = 0;
float* last_values;

int16_t clip(float in){

    if(in > INT16_MAX)
        return INT16_MAX;
    else if (in < INT16_MIN)
        return INT16_MIN;
    else
        return (int16_t)in;
}

float setAlphaForLowPass(float cutoffFrequency){
    float dt = 1.0f/SAMPLE_RATE;
    float two_pi_dt_fc = 2*M_PI*dt*cutoffFrequency;
    alpha = (two_pi_dt_fc)/(two_pi_dt_fc+1);
    return alpha;
}

float setAlphaForHighPass(float cutoffFrequency){
    float dt = 1.0f/SAMPLE_RATE;
    float two_pi_dt_fc = 2*M_PI*dt*cutoffFrequency;
    alpha = 1/(two_pi_dt_fc+1);
    return alpha;
}

float* constructDelayBuffer(size_t newN){
    N = newN;
    last_values = (float*)calloc(N, sizeof(float));
    return last_values;
}

int16_t low_pass(int16_t isample){

    static float last_y = 0.0f;
    float x = isample;
    float y = last_y + alpha * (x - last_y);  
    last_y = y;
    return clip(y);
}

int16_t high_pass(int16_t isample){

    static float last_y = 0.0f;
    static float last_x = 0.0f;
    float x = isample;
    float y = alpha * (last_y + x - last_x);  
    last_y = y;
    last_x = x;
    return clip(y);
}

int16_t distorsion(int16_t isample){

    float x = (float)isample / (float)INT16_MAX;
    float y = tanh(8*x);
    return clip(y*INT16_MAX);
}

int16_t delay(int16_t isample){
    static size_t index = 0;
    
    float x = isample;
    float y = alpha * (last_values[index]-x) + x;
    last_values[index] = y;
    index = (index + 1) % N;

    return clip(y);
    
}

int main(int argc, char *argv[]){

    int16_t (*effect_fcn)(int16_t);
    if (argc < 4){
        exit(1);
    }
    else{
        char* effect_type = argv[3];
        printf("Effect: ");
        if(strstr(effect_type, "disto") == effect_type){
            effect_fcn = &distorsion; 
            printf("Distorsion\n");
        }
        else if (strstr(effect_type, "low") == effect_type){
            float cutoff = atof(argv[4]);
            setAlphaForLowPass(cutoff);
            effect_fcn = &low_pass;
            printf("Low pass\n");
        }
        else if (strstr(effect_type, "high") == effect_type){
            float cutoff = atof(argv[4]);
            setAlphaForHighPass(cutoff);
            effect_fcn = &high_pass;
            printf("High Pass\n");
        }
        else if (strstr(effect_type, "delay") == effect_type){
            alpha = atof(argv[4]);
            N = atoi(argv[5]);
            constructDelayBuffer(N);
            effect_fcn = &delay;
            printf("Delay\n");
        }
        else{
            printf("Unknown filter \"%s\"", effect_type);
        }
    }

    printf("in: %s\n", argv[1]);
    printf("out: %s\n", argv[2]);
    int writer = setr_fifo_writer(argv[2]);
    int reader = setr_fifo_reader(argv[1]);

    int16_t samples[BUFFER_SIZE];
    char* buffer = (char*) samples;

    while (1){
        for (int i = 0; i<BUFFER_SIZE*2;){
            int data_read = 0;
            if ((data_read = read(reader, buffer+i, (BUFFER_SIZE*2)-i)) > 0){
                i += data_read;
            }
        }

        for (int i = 0; i<BUFFER_SIZE; i++){
            samples[i] = effect_fcn(samples[i]);
        }
        if (write(writer, buffer, BUFFER_SIZE*2) < 0){
            printf("Write error!\n");
            exit(1);
        }
    }

}
