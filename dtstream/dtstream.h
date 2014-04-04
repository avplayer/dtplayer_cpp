#ifndef DTSTREAM_H
#define DTSTREAM_H

#include "dt_av.h"
#include "dtstream_api.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

typedef enum{
    STREAM_INVALID = -1,
    STREAM_FILE,
    STREAM_FFMPEG,
    STREAM_UNSUPPORT
}stream_format_t;

typedef struct{
    int is_stream;
    int seek_support;
    int64_t cur_pos;
    int64_t stream_size;
    int eof_flag;
}stream_ctrl_t;

typedef struct stream_wrapper
{
    const char *name;
    int id;
	
	std::function<int (struct stream_wrapper * wrapper, char *stream_name)>open;
	std::function<int (struct stream_wrapper * wrapper, uint8_t *buf,int len)>read;
	std::function<int (struct stream_wrapper * wrapper, int64_t pos, int whence)>seek;
	std::function<int (struct stream_wrapper * wrapper)>close;
	
    void *stream_priv;          // point to priv context
    void *parent;               // point to parent, dtstream_context_t
    stream_ctrl_t info;
    struct stream_wrapper *next;
	
	template<typename OPEN, typename READ, typename SEEK, typename CLOSE>
	stream_wrapper(int _id, const char *_name, OPEN _open, READ _read, SEEK _seek, CLOSE _close)
	              :id(_id), name(_name), open(_open), read(_read), seek(_seek), close(_close)
	{}
	
} stream_wrapper_t;

typedef struct dtstream_context
{
	dtstream_para_t para;
    stream_wrapper_t *stream;
    void *parent;
	
	dtstream_context(dtstream_para_t &_para);
	int stream_open ();
	int stream_eof ();
	int64_t stream_tell ();
	int64_t stream_get_size ();
	int stream_read (uint8_t *buf,int len);
	int stream_seek (int64_t pos,int whence);
	int stream_close ();
	
} dtstream_context_t;

void stream_register_all();


#endif
