#ifndef APM_NS_H
#define APM_NS_H
#include <vector>
// implement noise suppression module

typedef struct NsHandleT ApmNsHandle;
typedef void ApmAgcHandle;

enum {
	NS_Mode_Mild = 0,
	Ns_Mode_Mideum,
	Ns_Mode_Aggressive,
};

class APM_NS{
public:
	APM_NS(){}
	~APM_NS();
	/*
	* initNsModule
	* Input:
	*     - frequency	   : sample rate
	*	  - ns_mode		   : NS_Mode_Mild (6dB), Ns_Mode_Mideum (10dB), Ns_Mode_Aggressive (15dB)
	*     - input_frames   : input frames size
	*     - input_channels : input channels
    *     - doAgc          : whether do agc before noise reduction
	* Return value : 0  - ok
	*                -1 - error
	*/
	bool initNsModule(unsigned int frequency, int ns_mode, 
                      int input_frames, int input_channels,
                      bool doAgc);

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
	std::vector<ApmNsHandle *> m_nsHandles;
    std::vector<ApmAgcHandle *> m_agcHandles;
    std::vector<int> m_captureLevel;
	void *capture_buffer = nullptr;
	short **channels_ptr_i = nullptr;
	unsigned int m_frequency;
	int m_channels;
	int m_ns_mode;
    bool m_hasInit = false;
    bool m_doAgc = false;
};


#endif
