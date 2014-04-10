#ifndef DTPORT_API_H
#define DTPORT_API_H

#include "dt_av.h"

typedef struct
{
    int has_video;
    int has_audio;
    int has_subtitle;
} dtport_para_t;

class dtport
{
public:
	dtport(){};
	std::function<int (dtport_para_t * para)>init;
	std::function<int ()>start;
	std::function<int ()>stop;
	std::function<int (dt_av_frame_t * frame, int type)>read_frame;
	std::function<int (dt_av_frame_t * frame, int type)>write_frame;
	std::function<int (buf_state_t * buf_state, int type)>get_state;
};

struct dtport_context;
class dthost;

class module_port
{
public:
	dtport *port_ext;
	struct dtport_context *pctx;
	dthost *host_ext;
	
	module_port(){};
	int dtport_init (dtport_para_t * para);
	int dtport_stop ();
	int dtport_write_frame ( dt_av_frame_t * frame, int type);
	int dtport_read_frame (dt_av_frame_t * frame, int type);
	int dtport_get_state (buf_state_t * buf_state, int type);
};

dtport *open_port_module();

#endif
