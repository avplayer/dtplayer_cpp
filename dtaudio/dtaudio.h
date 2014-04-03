#ifndef DT_AUDIO_H
#define DT_AUDIO_H

#include <thread>

#include "dt_av.h"
#include "dt_utils.h"
#include "dt_event.h"
#include "dtaudio_api.h"

#include "dtaudio_decoder.h"
#include "dtaudio_filter.h"
#include "dtaudio_output.h"

#include <unistd.h>

#define DTAUDIO_LOG_TAG "dtaudio"
#define DTAUDIO_PCM_BUF_SIZE 5*1024*1024; // pcm tmp buffer
#define MAX_DECODED_OUT_SIZE 19200 //max out size for decode one time

typedef struct
{
    int audio_decoder_format;
    int audio_filter_flag;
    int audio_output_id;
} dtaudio_ctrl_t;

typedef enum
{
    AUDIO_STATUS_IDLE,
    AUDIO_STATUS_INITING,
    AUDIO_STATUS_INITED,
    AUDIO_STATUS_RUNNING,
    AUDIO_STATUS_ACTIVE,
    AUDIO_STATUS_PAUSED,
    AUDIO_STATUS_STOP,
    AUDIO_STATUS_TERMINATED,
} dtaudio_status_t;

typedef struct dtaudio_context
{
    /*param */
    dtaudio_para_t audio_param;
    /*ctrl */
    dtaudio_ctrl_t audio_ctrl;
    /*buf */
    //dt_packet_list_t *audio_pack_list;
    dt_buffer_t audio_decoded_buf;
    dt_buffer_t audio_filtered_buf;
    /*module */
    dtaudio_decoder_t *audio_dec;
    dtaudio_filter_t *audio_filt;
    dtaudio_output_t *audio_out;
    //dtaudio_pts_t    audio_pts;
    /*other */
    std::thread event_loop_thread;
    dtaudio_status_t audio_state;
    void *audio_server;
    void *dtport_priv;          //data source
    void *parent;               //dtcodec
    
public:
	dtaudio_context(dtaudio_para_t &para);
#if 0
	dtaudio_context(dtaudio_para_t &para)
	{
		audio_param.channels = para.channels;
		audio_param.samplerate = para.samplerate;
		audio_param.dst_channels = para.dst_channels;
		audio_param.dst_samplerate = para.dst_samplerate;
		audio_param.data_width = para.data_width;
		audio_param.bps = para.bps;
		audio_param.afmt = para.afmt;
		
		audio_param.den = para.den;
		audio_param.num = para.num;
		
		audio_param.extradata_size = para.extradata_size;
		if(para.extradata_size > 0)
			;//audio_param.extradata = para.extradata;
		
		audio_param.audio_filter = para.audio_filter;
		audio_param.audio_output = para.audio_output;
		audio_param.avctx_priv = para.avctx_priv;
	}
#endif
	int64_t audio_get_current_pts ();
	int64_t audio_get_first_pts ();
	int audio_drop (int64_t target_pts);
	
	int audio_get_dec_state (dec_state_t * dec_state);
	int audio_get_out_closed ();
	int audio_start ();
	int audio_pause ();
	int audio_resume ();
	int audio_stop ();
	int audio_init ();

} dtaudio_context_t;

void audio_register_all();
void audio_update_pts (void *priv);
int audio_read_frame (void *priv, dt_av_frame_t * frame);
int audio_output_read (void *priv, uint8_t * buf, int size);

#endif
