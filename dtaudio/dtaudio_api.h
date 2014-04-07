#ifndef DTAUDIO_API_H
#define DTAUDIO_API_H

#include "dt_state.h"
#include <stdint.h>

#include <functional>

#define AUDIO_EXTRADATA_SIZE 4096

class dthost;

typedef struct dtaudio_para
{
    int channels,dst_channels;
    int samplerate,dst_samplerate;
    int data_width;
    int bps;
    int num, den;
    int extradata_size;
    unsigned char extradata[AUDIO_EXTRADATA_SIZE];
    int afmt;
    int audio_filter;
    int audio_output;
    void *avctx_priv;           //point to avcodec_context    
} dtaudio_para_t;

class dtaudio
{
public:
	dtaudio();
	std::function<int (dtaudio_para_t *para, dthost *host)>init;
	std::function<int ()>start;
	std::function<int ()>pause;
	std::function<int ()>resume;
	std::function<int ()>stop;
	std::function<int ()>get_pts;
	std::function<int (void *priv,int64_t target)>drop;
	std::function<int ()>get_first_pts;
	std::function<int (dec_state_t * dec_state)>get_state;
	std::function<int ()>get_out_closed;
};

dtaudio * open_audio_module();

#endif
