#ifndef DTVIDEO_API_H
#define DTVIDEO_API_H

#include "dt_state.h"
#include <stdint.h>

#include <functional>

#define VIDEO_EXTRADATA_SIZE 4096

class dthost;

typedef struct
{
    int vfmt;
    int d_width;                //dest w
    int d_height;               //dest h
    int s_width;                //src w
    int s_height;               //src h
    int d_pixfmt;               //dest pixel format
    int s_pixfmt;               //src pixel format
    int rate;
    int ratio;
    double fps;
    int num, den;               //for pts calc
    int extradata_size;
    unsigned char extradata[VIDEO_EXTRADATA_SIZE];
    int video_filter;
    int video_output;
    void *avctx_priv;
} dtvideo_para_t;

class dtvideo
{
public:
	dtvideo(){};
	std::function<int (dtvideo_para_t * para, dthost *host)>init;
	std::function<int ()>start;
	std::function<int ()>pause;
	std::function<int ()>resume;
	std::function<int ()>stop;
	std::function<int ()>get_pts;
	std::function<int (int64_t target)>drop;
	std::function<int ()>get_first_pts;
	std::function<int (dec_state_t * dec_state)>get_state;
	std::function<int ()>get_out_closed;
};

struct dtvideo_context;

class module_video
{
public:
	dtvideo *video_ext;
	struct dtvideo_context *vctx;
	dthost *host_ext;
	int dtvideo_init (dtvideo_para_t * para, dthost *host);
	int dtvideo_start ();
	int dtvideo_pause ();
	int dtvideo_resume ();
	int dtvideo_stop ();
	int64_t dtvideo_get_pts ();
	int64_t dtvideo_get_first_pts ();
	int dtvideo_drop (int64_t target_pts);
	int dtvideo_get_state (dec_state_t * dec_state);
	int dtvideo_get_out_closed ();
};

dtvideo *open_video_module();


#endif
