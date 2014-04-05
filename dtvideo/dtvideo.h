#ifndef DTVIDEO_H
#define DTVIDEO_H

#include "dt_av.h"
#include "dtvideo_api.h"
#include "dt_buffer.h"
#include "dt_queue.h"
#include "dtvideo_decoder.h"
#include "dtvideo_output.h"

#include <unistd.h>

#define DTVIDEO_BUF_SIZE 1024*1024

typedef enum
{
    VIDEO_STATUS_IDLE,
    VIDEO_STATUS_INITING,
    VIDEO_STATUS_INITED,
    VIDEO_STATUS_RUNNING,
    VIDEO_STATUS_ACTIVE,
    VIDEO_STATUS_PAUSED,
    VIDEO_STATUS_STOPPED,
    VIDEO_STATUS_TERMINATED,
} dtvideo_status_t;

typedef struct dtvideo_context
{
    dtvideo_para_t video_para;    
	
    dtvideo_decoder_t *video_dec;
    dtvideo_output_t *video_out;
    queue_t *vo_queue;

    /*pts*/
    int64_t first_pts;
    int64_t current_pts;
    int64_t last_valid_pts;
    dtvideo_status_t video_status;

    int event_loop_id;
    void *video_server;
    void *parent;               //dthost
    
    dtvideo_context(dtvideo_para_t &para);
	int64_t video_get_current_pts ();
	int64_t video_get_first_pts ();
	int video_drop (int64_t target_pts);

	int video_get_dec_state (dec_state_t * dec_state);
	int video_get_out_closed ();
	int video_start ();
	int video_pause ();
	int video_resume ();
	int video_stop ();
	int video_init ();
    
} dtvideo_context_t;

void video_register_all();
int dtvideo_read_frame (void *priv, dt_av_frame_t * frame);
AVPicture_t *dtvideo_output_read (void *priv);
AVPicture_t *dtvideo_output_pre_read (void *priv);
int dtvideo_get_avdiff (void *priv);
int64_t dtvideo_get_systime (void *priv);
void dtvideo_update_systime (void *priv, int64_t sys_time);
void dtvideo_update_pts (void *priv);



#endif
