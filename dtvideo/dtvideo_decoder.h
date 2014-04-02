#ifndef DTVIDEO_DECODER_H
#define DTVIDEO_DECODER_H

#include <functional>

#include "dt_buffer.h"
#include "dt_av.h"
#include "dtvideo_api.h"

#include <thread>

typedef enum
{
    VDEC_STATUS_IDLE,
    VDEC_STATUS_RUNNING,
    VDEC_STATUS_PAUSED,
    VDEC_STATUS_EXIT
} vdec_status_t;

typedef struct dtvideo_decoder dtvideo_decoder_t;

typedef struct video_decoder_operation
{

    const char *name;
    video_format_t vfmt;        // not used, for ffmpeg
    int type;
    
    std::function<int(dtvideo_decoder_t * decoder)> init;
    std::function<int(dtvideo_decoder_t * decoder, dt_av_frame_t * frame, AVPicture_t ** pic)> decode_frame;
    std::function<int(dtvideo_decoder_t * decoder)> release;
    
    template<typename INIT, typename DECODE, typename RELEASE>
    video_decoder_operation(int _type, const char * _name, video_format_t _vfmt,
	INIT _init, DECODE _decode, RELEASE _release)
       : name(_name), type(_type), vfmt(_vfmt), init(_init), decode_frame(_decode), release(_release)
	{
	}

} dec_video_wrapper_t;

struct dtvideo_decoder
{
    dtvideo_para_t para;
    dec_video_wrapper_t *dec_wrapper;
    std::thread video_decoder_thread;
    vdec_status_t status;
    int decode_err_cnt;

    int64_t pts_current;
    int64_t pts_first;
    unsigned int pts_last_valid;
    unsigned int pts_buffer_size;
    unsigned int pts_cache_size;
    int frame_count;

    dt_buffer_t *buf_out;
    void *parent;
    void *decoder_priv;
};

void vdec_register_all();
int video_decoder_init (dtvideo_decoder_t * decoder);
int video_decoder_release (dtvideo_decoder_t * decoder);
int video_decoder_stop (dtvideo_decoder_t * decoder);
int video_decoder_start (dtvideo_decoder_t * decoder);
void dtpicture_free (void *pic);

#endif
