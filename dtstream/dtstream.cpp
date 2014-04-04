#include "dtstream.h"

#include <unistd.h>
#include <vector>

#define TAG "STREAM"

#define REGISTER_STREAM(X,x) \
    {                         \
        extern stream_wrapper_t stream_##x; \
        register_stream(stream_##x);     \
    }
static std::vector<stream_wrapper_t> g_stream;

static void register_stream (stream_wrapper_t &wrapper)
{
	g_stream.push_back(wrapper);
	dt_info (TAG, "[%s:%d] register stream, name:%s fmt:%d \n", __FUNCTION__, __LINE__, wrapper.name, wrapper.id);
}

void stream_register_all ()
{
    REGISTER_STREAM (FILE, file);
#ifdef ENABLE_STREAM_FFMPEG
    REGISTER_STREAM (FFMPEG, ffmpeg);
#endif
}

static int get_stream_id(char *name)
{
    int ret = access(name,0);
    if(ret == 0)
        return STREAM_FILE;
    return STREAM_FFMPEG; // default 
}

static int stream_select (dtstream_context_t * stm_ctx)
{	
	if (g_stream.empty())
    {
        dt_error (TAG, "[%s:%d] select no stream \n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    int id = get_stream_id(stm_ctx->para.stream_name);
    std::vector< stream_wrapper_t >::iterator it = g_stream.begin();

	for (std::vector< stream_wrapper_t >::iterator it = g_stream.begin(); it != g_stream.end(); it++)
	{
		if ((it->id ==  id) || (it->id == STREAM_FFMPEG))
		{
			stm_ctx->stream = &(*it);
		    dt_info(TAG,"SELECT VO:%s \n",stm_ctx->stream->name);
			return 0;
		}
	}
	dt_info (TAG, "[%s:%d] select stream, name:%s id:%d \n", __FUNCTION__, __LINE__, it->name, it->id);
    return 0;
	
}

dtstream_context::dtstream_context(dtstream_para_t &_para)
{
	para.stream_name = _para.stream_name;
	stream = nullptr;
}


int dtstream_context::stream_open ()
{
    int ret = 0;
    if (stream_select (this) == -1)
    {
        dt_error (TAG, "select stream failed \n");
        return -1;
    }
    stream_wrapper_t *wrapper = this->stream;
    memset(&wrapper->info,0,sizeof(stream_ctrl_t));
    dt_info (TAG, "select stream:%s\n", wrapper->name);
    ret = wrapper->open (wrapper, this->para.stream_name);
    if (ret < 0)
    {
        dt_error (TAG, "stream open failed\n");
        return -1;
    }
    dt_info (TAG, "stream open ok\n");
    return 0;
}

int dtstream_context::stream_eof ()
{
    stream_wrapper_t *wrapper = this->stream;
    stream_ctrl_t *info = &wrapper->info;
    return info->eof_flag;
}

int64_t dtstream_context::stream_tell ()
{
    stream_wrapper_t *wrapper = this->stream;
    stream_ctrl_t *info = &wrapper->info;
    return info->cur_pos;
}

int64_t dtstream_context::stream_get_size ()
{
    stream_wrapper_t *wrapper = this->stream;
    stream_ctrl_t *info = &wrapper->info;
    return info->stream_size;
}

int dtstream_context::stream_read (uint8_t *buf,int len)
{
    stream_wrapper_t *wrapper = this->stream;
    return wrapper->read(wrapper,buf,len);
}

int dtstream_context::stream_seek (int64_t pos,int whence)
{
    stream_wrapper_t *wrapper = this->stream;
    return wrapper->seek(wrapper,pos,whence);
}

int dtstream_context::stream_close ()
{
    stream_wrapper_t *wrapper = this->stream;
    wrapper->close(wrapper);
    return 0;
}
