#ifndef DEMUXER_CTRL_H
#define DEMUXER_CTRL_H

#include "dt_media_info.h"
#include "dtstream_api.h"
#include "dt_av.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#define PROBE_BUF_SIZE 1024*1024 //1M

typedef enum{
    DEMUXER_INVALID = -1,
    DEMUXER_AAC,
    DEMUXER_FFMPEG,
    DEMUXER_UNSUPPORT,
}demuxer_format_t;

typedef struct demuxer_wrapper
{
    const char *name;
    int id;
    
    std::function<int (struct demuxer_wrapper *wrapper, void *parent)>probe;
	std::function<int (struct demuxer_wrapper *wrapper)>open;
	std::function<int (struct demuxer_wrapper *wrapper, dt_av_frame_t * frame)>read_frame;
	std::function<int (struct demuxer_wrapper *wrapper, dt_media_info_t * info)>setup_info;
	std::function<int (struct demuxer_wrapper *wrapper, int timestamp)>seek_frame;
	std::function<int (struct demuxer_wrapper *wrapper)>close;
	
	void *demuxer_priv;         // point to priv context
    void *parent;               // point to parent, dtdemuxer_context_t
	
	template <typename PROBE, typename OPEN, typename READ, typename SETUP, typename SEEK, typename CLOSE>
	demuxer_wrapper(int _id, const char * _name, PROBE _probe, OPEN _open, READ _read, SETUP _setup, SEEK _seek, CLOSE _close)
					:id(_id), name(_name), probe(_probe), open(_open), read_frame(_read), setup_info(_setup), seek_frame(_seek), close(_close)
	{}
    
} demuxer_wrapper_t;

typedef struct
{
    char *file_name;
    dt_media_info_t media_info;
    demuxer_wrapper_t *demuxer;
    dt_buffer_t probe_buf;    
    void *stream_priv;
    void *parent;
} dtdemuxer_context_t;

void demuxer_register_all();
int demuxer_open (dtdemuxer_context_t * dem_ctx);
int demuxer_read_frame (dtdemuxer_context_t * dem_ctx, dt_av_frame_t * frame);
int demuxer_seekto (dtdemuxer_context_t * dem_ctx, int timestamp);
int demuxer_close (dtdemuxer_context_t * dem_ctx);

#endif
