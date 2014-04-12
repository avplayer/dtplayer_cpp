#ifndef DTSTREAM_API_H
#define DTSTREAM_API_H

#include "dt_av.h"
#include <functional>

typedef struct
{
    char *stream_name;
} dtstream_para_t;



class dtstream
{
public:
	dtstream(){};
	std::function<int (dtstream_para_t * para)>open;
	std::function<int ()>eof;
	std::function<int ()>tell;
	std::function<int ()>get_size;
	std::function<int (int64_t size)>skip;
	std::function<int (uint8_t *buf,int len)>read;
	std::function<int (int64_t pos,int whence)>seek;
	std::function<int ()>close;
};

struct dtstream_context;

class module_stream
{
public:
	dtstream *stream_ext;
	struct dtstream_context *stm_ctx;
	
	module_stream(){};
	int dtstream_open (dtstream_para_t * para);
	int dtstream_eof ();
	int64_t dtstream_tell ();
	int64_t dtstream_get_size ();
	int dtstream_skip (int64_t size);
	int dtstream_read (uint8_t *buf,int len);
	int dtstream_seek (int64_t pos,int whence);
	int dtstream_close ();
};

dtstream *open_stream_module();

#endif
