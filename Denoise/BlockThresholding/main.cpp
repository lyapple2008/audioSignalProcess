#include "../../common/wav_parser/wav_io.h"
#include "../../common/jansson/include/jansson.h"

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
	FILE *pInFile = fopen(infile, "rb");
	FILE *pOutFile = fopen(outfile, "wb");
	if (!pInFile || !pOutFile) {
		fprintf(stderr, "error: can not open file!!!\n");
		return 1;
	}
	WAV_HEADER wavHeader;
	read_header(&wavHeader, pInFile);
	write_header(&wavHeader, pOutFile);


	return 0;
}