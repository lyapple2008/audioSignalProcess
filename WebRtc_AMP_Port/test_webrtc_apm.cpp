#include <stdio.h>
#include "webrtc\modules\audio_processing\audio_processing_impl.h"
#include "webrtc\common.h" //for class Config
extern "C"{
#include "wav_io.h"
}
#define NN	480 //48k/100=10ms

int main(int argc, char *argv[])
{
	if (argc != 4){
		printf("Usage: exe echo_file ref_file result_file\n");
		return -1;
	}
	short echo_buf[NN];
	short ref_buf[NN];
	short result_buf[NN];
	FILE *echo_file, *ref_file, *result_file;
	WAV_HEADER echo_header, ref_header;
	if (fopen_s(&echo_file, argv[1], "rb") ||
		fopen_s(&ref_file, argv[2], "rb") ||
		fopen_s(&result_file, argv[3], "rb")){
		printf("Fail to open file!!!\n");
		return -1;
	}
	if (read_header(&echo_header, echo_file) != 0){
		printf("Fail to parse wav file: %s\n", argv[1]);
		return -1;
	}
	if (read_header(&ref_header, ref_file) != 0){
		printf("Fail to parse wav file: %s\n", argv[2]);
		return -1;
	}
	if (echo_header.format.sample_per_sec != ref_header.format.sample_per_sec ||
		echo_header.format.bits_per_sample != ref_header.format.bits_per_sample ||
		echo_header.format.channels != ref_header.format.channels){
		printf("The format of echo and ref file are not the same!\n");
		return -1;
	}
	write_header(&echo_header, result_file);

	// Initialize
	webrtc::Config config;
	//webrtc::AudioFrame far_frame;
	//webrtc::AudioFrame near_frame;
	webrtc::AudioProcessingImpl amp(config);
	int sample_rate = echo_header.format.sample_per_sec;
	int channels = echo_header.format.channels;
	amp.Initialize(sample_rate, sample_rate, sample_rate, 
		webrtc::AudioProcessing::kMono, webrtc::AudioProcessing::kMono, webrtc::AudioProcessing::kMono);
	amp.echo_cancellation()->Enable(true);


	return 0;
}