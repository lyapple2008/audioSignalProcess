#ifndef APM_NS_H
#define APM_NS_H
#include <vector>
// implement noise suppression module

typedef struct NsHandleT NsHandle;

enum {
	NS_Mode_Mild = 0,
	Ns_Mode_Mideum,
	Ns_Mode_Aggressive,
};

class APM_NS{
public:
	APM_NS():capture_buffer(0),
			 channels_ptr_i(0),
			 init_flag(false){}
	~APM_NS();
	/*
	* initNsModule
	* Input:
	*     - frequency	   : sample rate
	*	  - ns_mode		   : NS_Mode_Mild (6dB), Ns_Mode_Mideum (10dB), Ns_Mode_Aggressive (15dB)
	*     - input_frames   : input frames size
	*     - input_channels : input channels
	* Return value : 0  - ok
	*                -1 - error
	*/
	bool initNsModule(unsigned int frequency, int ns_mode, int input_frames, int input_channels);

	/*
	* processCaptureStream
	* Input:
	*     - data				: interleave samples
	*	  - samples_per_channel	: the number of samples in each channel
	*     - channels		    : the number of channels
	* Output:
	*     - data                : the denoise process will do in place
	* Return value : 0  - ok
	*                -1 - error
	*/
	void processCaptureStream(float* data, int samples_per_channel, int input_channels);
	void processCaptureStream(short* data, int samples_per_channel, int input_channels);
private:
	std::vector<NsHandle *> m_handles;
	void *capture_buffer;
	short **channels_ptr_i;
	unsigned int m_frequency;
	int m_channels;
	int m_ns_mode;
	bool init_flag;//ns module has been initialed successfully
};


#endif
