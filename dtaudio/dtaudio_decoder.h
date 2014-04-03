#ifndef DT_AUDIO_DECODER_H
#define DT_AUDIO_DECODER_H

#include "dt_buffer.h"
#include "dt_av.h"
#include "dtaudio_api.h"
#include <functional>

#include <thread>

typedef enum
{
    ADEC_STATUS_IDLE,
    ADEC_STATUS_RUNNING,
    ADEC_STATUS_PAUSED,
    ADEC_STATUS_EXIT
} adec_status_t;

typedef struct dtaudio_decoder dtaudio_decoder_t;

typedef struct{
    uint8_t *inptr;
    int inlen;
    int consume;
    uint8_t *outptr;
    int outsize; // buffer size
    int outlen;  // buffer level

    int info_change;
    int channels;
    int samplerate;
    int bps;
}adec_ctrl_t;

typedef struct dec_audio_wrapper
{

    const char *name;
    audio_format_t afmt;        //not used, for ffmpeg
    int type;
    
    std::function<int (struct dec_audio_wrapper * wrapper,void *parent)> init;
    std::function<int (struct dec_audio_wrapper * wrapper, adec_ctrl_t *pinfo)> decode_frame;
    std::function<int (struct dec_audio_wrapper * wrapper)> release;
    
    void *adec_priv;
    void *parent;

    template<typename INIT, typename DECODE, typename RELEASE>
    dec_audio_wrapper(int _type, const char * _name, audio_format_t _afmt,
		INIT _init, DECODE _decode, RELEASE _release)
		  : name(_name), type(_type), afmt(_afmt), init(_init), decode_frame(_decode), release(_release)
	{
	}
} dec_audio_wrapper_t;

struct dtaudio_decoder
{
    dtaudio_para_t aparam;
    dec_audio_wrapper_t *dec_wrapper;
	std::thread audio_decoder_thread;
    adec_status_t status;
    int decode_err_cnt;
    int decode_offset;

    int64_t pts_current;
    int64_t pts_first;
    int64_t pts_last_valid;
    int pts_buffer_size;
    int pts_cache_size;
    
    adec_ctrl_t info;
    dt_buffer_t *buf_out;
    void *parent;
    void *decoder_priv;         //point to avcodeccontext
    
    dtaudio_decoder(dtaudio_para_t &para);
	
	int audio_decoder_init ();
	int audio_decoder_release ();
	int audio_decoder_stop ();
	int audio_decoder_start ();
	int64_t audio_decoder_get_pts ();
	
};

void adec_register_all();

#endif
