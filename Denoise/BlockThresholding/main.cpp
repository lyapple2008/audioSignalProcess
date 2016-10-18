#include "../../common/jansson/include/jansson.h"

extern "C"{
#include "../../common/wav_parser/wav_io.h"
#include "src\audioDenoiseBlockTreshold.h"
}


int main(int argc, char *argv[])
{
    //-------------Parse config
    const char *config_file = "config.jason";
    json_t *config;
    json_error_t error;

    config = json_load_file(config_file, 0, &error);
    if (!config) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 1;
    }

    if (!json_is_object(config)) {
        fprintf(stderr, "error: config is not an object\n");
        json_decref(config);
        return 1;
    }

    json_t *json_infile = json_object_get(config, "infile");
    if (!json_infile) {
        fprintf(stderr, "error: there is not infile!\n");
        return 1;
    }
    const char *infile = json_string_value(json_infile);
    json_t *json_outfile = json_object_get(config, "outfile");
    if (!json_outfile) {
        fprintf(stderr, "error: there is not outfile!\n");
        return 1;
    }
    const char *outfile = json_string_value(json_outfile);

    json_t *json_frameMs = json_object_get(config, "frame_ms");
    json_int_t frameMs = json_integer_value(json_frameMs);
    
    //-----------------Parse wav
    FILE *pInFile, *pOutFile;
    fopen_s(&pInFile, infile, "rb");
    fopen_s(&pOutFile, outfile, "wb");
    if (!pInFile || !pOutFile) {
        fprintf(stderr, "error: can not open file!!!\n");
        return 1;
    }
    WAV_HEADER wavHeader;
    read_header(&wavHeader, pInFile);
    write_header(&wavHeader, pOutFile);

    int32_t ret;
    int32_t time_win = frameMs;
    int32_t freq = wavHeader.format.sample_per_sec;
    int32_t channels = wavHeader.format.channels;
    int32_t bitsPerSample = wavHeader.format.bits_per_sample;
    int32_t num_samples = wavHeader.data.size / channels / bitsPerSample / 8;
    MarsBlockThreshold_t denoise_handle;
    ret = blockThreshold_init(&denoise_handle, time_win, freq);
    if (ret != MARS_OK) {
        fprintf(stderr, "error: blockThreshold_init\n");
        return -1;
    }
    int32_t frame_size = blockThreshold_samples_per_time(&denoise_handle);
    int16_t *inbuf = (int16_t *)malloc(sizeof(int16_t) * frame_size);
    int32_t outbuf_len = blockThreshold_max_output(&denoise_handle);
    int16_t *outbuf = (int16_t *)malloc(sizeof(int16_t) * outbuf_len);
    if (!inbuf || !outbuf) {
        fprintf(stderr, "Error in malloc!!!\n");
        return -1;
    }
    
    int32_t readed = 0;
    int32_t frm_cnt = 0;
    while (num_samples > frame_size) {
        readed = fread(inbuf, sizeof(int16_t), frame_size, pInFile);
        if (readed != frame_size) {
            fprintf(stderr, "Error in read file!!!\n");
            return -1;
        }

        fprintf(stdout, "Frame: %d\n", frm_cnt++);
        ret = blockThreshold_denoise(&denoise_handle, inbuf, frame_size);
        
        if (ret == MARS_ERROR_PARAMS) {
            fprintf(stderr, "ret = MARS_ERROR_PARAMS\n");
            break;
        } else if (ret == MARS_NEED_MORE_SAMPLES) {
            continue;
        } else if (ret == MARS_CAN_OUTPUT) {
            int32_t len = blockThreshold_output(&denoise_handle, outbuf, outbuf_len);
            fwrite(outbuf, sizeof(int16_t), len, pOutFile);
            fprintf(stdout, "one macro block processed!!\n");
        }
    }

    free(inbuf);
    fclose(pInFile);
    fclose(pOutFile);
    blockThreshold_free(&denoise_handle);

    return 0;
}