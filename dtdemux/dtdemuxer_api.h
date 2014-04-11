#ifndef DTDEMUXER_API_H
#define DTDEMUXER_API_H

#include "dt_av.h"
#include "dt_media_info.h"

typedef struct
{
    char *file_name;
} dtdemuxer_para_t;


class dtdemux
{
public:
	dtdemux(){};
	std::function<int (dtdemuxer_para_t * para)>open;
	std::function<dt_media_info_t* ()>get_media_info;
	std::function<int (dt_av_frame_t * frame)>read_frame;
	std::function<int (int timestamp)>seekto;
	std::function<int ()>close;
};

struct dtdemuxer_context;

class module_demux
{
public:
	dtdemux *demux_ext;
	struct dtdemuxer_context *dem_ctx;
	module_demux(){};
	int dtdemuxer_open (dtdemuxer_para_t * para);
	dt_media_info_t *dtdemuxer_get_media_info ();
	int dtdemuxer_read_frame (dt_av_frame_t * frame);
	int dtdemuxer_seekto (int timestamp);
	int dtdemuxer_close ();
};

dtdemux *open_demux_module();

#endif
