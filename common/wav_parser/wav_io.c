#include "wav_io.h"
#include <string.h>

static int string_cmp(const char *str1, const char *str2, int str_len)
{
    while (str_len--){
        if (*str1++ != *str2++){
            return -1;
        }
    }
    return 0;
}

int search_ID(const char *ID, char *buf, int buf_size, int *loc)
{
    int id_len = strlen(ID);
    int num = 0;
    
    while (buf_size >= id_len){
        if (string_cmp(ID, buf, id_len) == 0){
            *loc = num;
            return 0;
        }
        num++;
        buf_size--;
        buf++;
    }

    return -1;
}

int read_header(WAV_HEADER *header, FILE *file)
{
    char buf[BUFFER_SIZE];
    int buf_len;
    int loc = 0;
    char *p_buf = buf;

    buf_len = fread(buf, 1, BUFFER_SIZE, file);
    if (buf_len <= 0){
        return -1;
    }
    //RIFF chunk
    if (search_ID("RIFF", p_buf, buf_len, &loc) == -1){
        return -1;
    }
    p_buf += loc;
    buf_len -= loc;
    if (buf_len < sizeof(RIFF_CHUNK)){
        return -1;
    }
    memcpy(&(header->riff), p_buf, sizeof(RIFF_CHUNK));
    if (//memcmp(header.riff.ID, "RIFF", 4)!=0 || 
        memcmp(header->riff.type, "WAVE", 4)!=0){
        return -1;
    }
    p_buf += sizeof(RIFF_CHUNK);
    buf_len -= sizeof(RIFF_CHUNK);
    //FMT chunk
    if (search_ID("fmt ", p_buf, buf_len, &loc) == -1){
        return -1;
    }
    p_buf += loc;
    buf_len -= loc;
    if (buf_len < sizeof(FORMAT_CHUNK)){
        return -1;
    }
    memcpy(&(header->format), p_buf, sizeof(FORMAT_CHUNK));
    p_buf += sizeof(FORMAT_CHUNK);
    buf_len -= sizeof(FORMAT_CHUNK);
    // DATA chunk
    if (search_ID("data", p_buf, buf_len, &loc) == -1){
        return -1;
    }
    p_buf += loc;
    buf_len -= loc;
    if (buf_len < sizeof(DATA_CHUNK)){
        return -1;
    }
    memcpy(&(header->data), p_buf, sizeof(DATA_CHUNK));
    buf_len -= sizeof(DATA_CHUNK);

    fseek(file, -buf_len, SEEK_CUR);
    return 0;
}

int write_header(WAV_HEADER *header, FILE *file)
{
    header->format.size = 16;
    fwrite(header, sizeof(WAV_HEADER), 1, file);

    return 0;
}

void print_header(WAV_HEADER *header)
{
    printf("RIFF_CHUNK:\n");
    printf("    ID: RIFF\n");
    printf("    SIZE: 0x%x\n", header->riff.size);
    printf("    TYPE: WAVE\n");
    printf("FORMAT_CHUNK:\n");
    printf("    ID: fmt \n");
    printf("    SIZE: %d\n", header->format.size);
    printf("    FormatTag:0x%x\n", header->format.format);
    printf("    Channels: %d\n", header->format.channels);
    printf("    SamplePerSec: %d\n", header->format.sample_per_sec);
    printf("    AvgBytesPerSec: %d\n", header->format.avg_bytes_per_sec);
    printf("    BlockAlign: %d\n", header->format.blockAlign);
    printf("    BitsPerSample: %d\n", header->format.bits_per_sample);
    printf("DATA_CHUNK:\n");
    printf("    ID: data\n");
    printf("    SIZE: 0x%x\n", header->data.size);
}

int read_samples(short *buf, int num_samples, WAV_HEADER *header, 
                         FILE *file)
{
    int len = fread(buf, sizeof(short), num_samples, file);
    return len;
}

int write_samples(short *buf, int num_samples, WAV_HEADER *header, 
                          FILE *file)
{
    int len = fwrite(buf, sizeof(short), num_samples, file);

    return len;
}