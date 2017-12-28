#include <string.h>
#include "webrtc/modules/audio_processing/agc/legacy/gain_control.h"
#include "webrtc/modules/interface/module_common_types.h"
#include "webrtc/modules/audio_processing/audio_buffer.h"
extern "C"{
#include "wav_io.h"
}

using namespace webrtc;

typedef void AGCHandle;

enum Level
{
    kLow,
    kModerate,
    kHigh,
    kVeryHigh
};

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Please input parameters\n");
        return -1;
    }

    printf("Process %s -> %s\n", argv[1], argv[2]);
    FILE *fr, *fw;
    if (fopen_s(&fr, argv[1], "rb") ||
        fopen_s(&fw, argv[2], "wb")){
        printf("Fail to open file!!!\n");
        return -1;
    }
    if (!fr || !fw)
    {
        printf("Can't open file!\n");
        return -1;
    }

    WAV_HEADER header;
    if (read_header(&header, fr) != 0){
        printf("Fail to read wav header!\n");
        return -1;
    } 

    print_header(&header);
    write_header(&header, fw);

    if (header.format.bits_per_sample != 16){
        printf("Now only support 16 bits per sample!\n");
        return -1;
    }

    uint32_t frequency = header.format.sample_per_sec;//16000;
    uint16_t length = frequency / 100;
    if (frequency == 8000) {
        length = 80;
    } else if (frequency == 16000 ||
               frequency == 32000 ||
               frequency == 48000) {
        length = 160;
    } else {
        printf("Only support sample rate: 8000, 16000, 32000, 48000\n");
        return -1;
    }
    int16_t channels = header.format.channels;

    if (channels > 1) {
        printf("Now only support single channel\n");
        return -1;
    }

    AGCHandle* handle = NULL;
    if (WebRtcAgc_Create(&handle) != 0) {
        printf("Fail to create agc handel\n");
        return -1;
    } 
    if (WebRtcAgc_Init(handle, 0, 255, kAgcModeAdaptiveDigital, frequency) != 0) {
        printf("Fail to init agc module\n");
        return -1;
    }

    int16_t *input = (int16_t *)new short[length];
    int16_t *output = (int16_t *)new short[length];

    memset(input, 0, length*sizeof(int16_t));
    memset(output, 0, length*sizeof(int16_t));

    AudioFrame capture_frame;
    AudioBuffer capture_buffer(length, channels, length, channels, length);
    int32_t frm_cnt = 0;
    int32_t micLevel = 0;
    int32_t micLevelIn = 0;
    int32_t micLevelOut = 0;
    
    while (!feof(fr))
    {
        //fread(input, sizeof(int16_t), length, fr);
        read_samples(input, length, &header, fr);
        capture_frame.UpdateFrame(0, 0, input, length, frequency,
            AudioFrame::kNormalSpeech, AudioFrame::kVadUnknown, channels);
        capture_buffer.DeinterleaveFrom(&capture_frame);

        int ret = WebRtcAgc_VirtualMic(
                    handle,
                    capture_buffer.split_bands(0),
                    capture_buffer.num_bands(),
                    static_cast<int16_t>(capture_buffer.samples_per_split_channel()),
                    micLevel,
                    &micLevelIn);

        if (ret != 0) {
            printf("Failure in WebRtcAgc_VirtualMic\n");
            return -1;
        }

        uint8_t stream_is_saturated = 0;
        ret = WebRtcAgc_Process(
                    handle,
                    capture_buffer.split_bands_const(0),
                    capture_buffer.num_bands(),
                    static_cast<int16_t>(capture_buffer.samples_per_split_channel()),
                    capture_buffer.split_bands(0),
                    micLevelIn,
                    &micLevelOut,
                    0,
                    &stream_is_saturated
        );
        if (ret != 0) {
            printf("Failure in WebRtcAgc_Process\n");
            return -1;
        }

        //fwrite(output, sizeof(int16_t), length, fw);
        capture_buffer.InterleaveTo(&capture_frame, true);
        write_samples(capture_frame.data_, length, &header, fw);

        printf("Frame #%d\n", frm_cnt++);
    }

    WebRtcAgc_Free(handle);

    fclose(fr);
    fclose(fw);

    delete[]input;
    delete[]output;

    return 0;
}

