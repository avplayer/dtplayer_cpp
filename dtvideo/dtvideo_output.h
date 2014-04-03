#ifndef DTVIDEO_OUTPUT_H
#define DTVIDEO_OUTPUT_H

#include "dtvideo_api.h"
#include "dt_av.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include <thread>

typedef struct dtvideo_output dtvideo_output_t;

typedef struct vo_wrapper
{
    int id;
    const char *name;
	
	std::function<int(struct vo_wrapper * wrapper, void *parent)> vo_init;
	std::function<int(struct vo_wrapper * wrapper)> vo_stop;
	std::function<int(struct vo_wrapper * wrapper, AVPicture_t * pic)> vo_render;
    void *handle;
	void *parent;
	
    template<typename INIT, typename RENDER, typename STOP>
    vo_wrapper(int _id, const char * _name,
	INIT _init, RENDER _render, STOP _stop)
       : name(_name), id(_id), vo_init(_init), vo_render(_render), vo_stop(_stop)
	{
	}
	
} vo_wrapper_t;

typedef enum
{
    VO_STATUS_IDLE,
    VO_STATUS_PAUSE,
    VO_STATUS_RUNNING,
    VO_STATUS_EXIT,
} vo_status_t;

typedef enum _VO_ID_
{
    VO_ID_EXAMPLE = -1,
    VO_ID_SDL,
    VO_ID_X11,
    VO_ID_FB,
    VO_ID_GL,
    VO_ID_DIRECTX,
    VO_ID_SDL2,
} dt_vo_t;

typedef struct
{
    int vout_buf_size;
    int vout_buf_level;
} vo_state_t;

struct dtvideo_output
{
    /*param */
    dtvideo_para_t para;
    vo_wrapper_t *wrapper;
    vo_status_t status;
    std::thread video_output_thread;
    vo_state_t state;

    uint64_t last_valid_latency;
    void *parent;               //point to dtaudio_t, can used for param of pcm get interface
};

void vout_register_all();
int video_output_init (dtvideo_output_t * vo, int vo_id);
int video_output_release (dtvideo_output_t * vo);
int video_output_stop (dtvideo_output_t * vo);
int video_output_resume (dtvideo_output_t * vo);
int video_output_pause (dtvideo_output_t * vo);
int video_output_start (dtvideo_output_t * vo);

uint64_t video_output_get_latency (dtvideo_output_t * vo);
int video_output_get_level (dtvideo_output_t * vo);
#endif
