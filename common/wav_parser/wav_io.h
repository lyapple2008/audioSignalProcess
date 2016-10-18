#ifndef WAV_PARSE_H
#define WAV_PARSE_H

    //for wav file input and output
#include <stdio.h>

#define BUFFER_SIZE 256
#define ID_LEN 4

    typedef struct {
        char ID[ID_LEN];
        int size;
        char type[ID_LEN];
    }RIFF_CHUNK;

    typedef struct {
        char ID[ID_LEN];
        int size;
        short format;
        short channels;
        int sample_per_sec;
        int avg_bytes_per_sec;
        short blockAlign;
        short bits_per_sample;
    }FORMAT_CHUNK;

    typedef struct {
        char ID[ID_LEN];
        int size;
    }DATA_CHUNK;

    typedef struct {
        RIFF_CHUNK riff;
        FORMAT_CHUNK format;
        DATA_CHUNK data;
    }WAV_HEADER;

    int search_ID(const char *ID, char *buf, int buf_size, int *loc);

    int read_header(WAV_HEADER *header, FILE *file);
    int write_header(WAV_HEADER *header, FILE *file);
    int read_samples(short *buf, int num_samples, WAV_HEADER *header, FILE *file);
    int write_samples(short *buf, int num_samples, WAV_HEADER *header, FILE *file);
    void print_header(WAV_HEADER *header);
    
    
#endif