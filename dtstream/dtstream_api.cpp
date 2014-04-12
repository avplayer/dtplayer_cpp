#include "dtstream_api.h"
#include "dtstream.h"

#define TAG "STREAM_API"

dtstream* open_stream_module()
{
	dtstream *stream = new dtstream;
	module_stream *mod_stream = new module_stream;
	
	stream->open = std::bind(&module_stream::dtstream_open,mod_stream,std::placeholders::_1);
	stream->eof = std::bind(&module_stream::dtstream_eof,mod_stream);
	stream->tell = std::bind(&module_stream::dtstream_tell,mod_stream);
	stream->get_size = std::bind(&module_stream::dtstream_get_size,mod_stream);
	stream->skip = std::bind(&module_stream::dtstream_skip,mod_stream,std::placeholders::_1);
	stream->read = std::bind(&module_stream::dtstream_read,mod_stream,std::placeholders::_1,std::placeholders::_2);
	stream->seek = std::bind(&module_stream::dtstream_seek,mod_stream,std::placeholders::_1,std::placeholders::_2);
	stream->close = std::bind(&module_stream::dtstream_close,mod_stream);
	
	mod_stream->stream_ext = stream;
    dt_info(TAG,"OPEN STREAM MODULE ok \n");
	return stream;
}

int module_stream::dtstream_open (dtstream_para_t * para)
{
	dtstream_para_t &spara = *para;
    stm_ctx = new dtstream_context(spara);
	stm_ctx->parent = this;
    if(stm_ctx->stream_open() == -1)
    {
        dt_error(TAG,"STREAM CONTEXT OPEN FAILED \n");
        free(stm_ctx);
        return -1;
    }
    dt_info(TAG,"STREAM CTX OPEN SUCCESS\n");
    return 0;
}

int64_t module_stream::dtstream_get_size()
{
    return stm_ctx->stream_get_size();
}

int module_stream::dtstream_eof ()
{
    return stm_ctx->stream_eof();
}

int64_t module_stream::dtstream_tell ()
{
    return stm_ctx->stream_tell();
}

/*
 * skip size byte 
 * maybe negitive , then seek forward
 *
 * */
int module_stream::dtstream_skip (int64_t size)
{
    return stm_ctx->stream_seek(size,SEEK_CUR);
}

int module_stream::dtstream_read (uint8_t *buf,int len)
{
    return stm_ctx->stream_read(buf,len);
}

int module_stream::dtstream_seek (int64_t pos ,int whence)
{
    return stm_ctx->stream_seek(pos,whence);
}

int module_stream::dtstream_close ()
{
    if(stm_ctx)
    {
        stm_ctx->stream_close();
        delete(stm_ctx);
    }
    return 0;
}
