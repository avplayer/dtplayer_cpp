#ifndef DT_HOST_H
#define DT_HOST_H

#include "dthost_api.h"
#include "dt_av.h"

#include <stdlib.h>

class dtaudio;
class dtvideo;
class dtport;

typedef struct dthost_context
{
    dthost_para_t para;
    /*av sync */
    dt_sync_mode_t av_sync;
    int64_t sys_time;
    int64_t pts_audio;
    int64_t pts_video;
    int sync_enable;
    int sync_mode;
    int64_t av_diff;
    /*a-v-s port part */
    dtport *port_ext;
	dtaudio *audio_ext;
	dtvideo *video_ext;
	module_host *parent;
	
	dthost_context(dthost_para_t &_para);
	int host_start ();
	int host_pause ();
	int host_resume ();
	int host_stop ();
	int host_init ();
	int host_write_frame (dt_av_frame_t * frame, int type);
	int host_read_frame (dt_av_frame_t * frame, int type);

	int host_sync_enable ();
	int64_t host_get_apts ();
	int64_t host_get_vpts ();
	int64_t host_get_systime ();
	int64_t host_get_avdiff ();
	int64_t host_get_current_time ();
	int host_update_apts (int64_t apts);
	int host_update_vpts (int64_t vpts);
	int host_update_systime (int64_t sys_time);
	int host_get_state (host_state_t * state);
	int host_get_out_closed ();
	
} dthost_context_t;

#endif
