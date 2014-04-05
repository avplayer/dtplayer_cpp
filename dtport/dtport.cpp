#include "dtport.h"

#define TAG "PORT-MGT"

dtport_context::dtport_context(dtport_para_t& para)
{
	param.has_audio = para.has_audio;
	param.has_subtitle = para.has_subtitle;
	param.has_video = para.has_video;
	
	this->status = PORT_STATUS_IDLE;
}



int dtport_context::port_init ()
{
    int ret = 0;
    if (this->param.has_audio == 0 && this->param.has_video == 0)
    {
        dt_error (TAG, "[%s:%d] no av stream found \n", __FUNCTION__, __LINE__);
        ret = -1;
    }
    return ret;
}

int dtport_context::port_write_frame (dt_av_frame_t * frame, int type)
{
    int audio_write_enable = 1;
    int video_write_enable = 1;
	
    if (this->param.has_audio)
        audio_write_enable = (this->queue_audio.size() < QUEUE_MAX_PACK_NUM);
    else
        audio_write_enable = 0;
    if (this->param.has_video)
        video_write_enable = (this->queue_video.size() < QUEUE_MAX_PACK_NUM);
    else
        video_write_enable = 0;

    if (!audio_write_enable && !video_write_enable)
    {
        dt_debug (TAG, "A-V HAVE ENOUGH BUFFER,DO NOT WRITE\n");
        return -1;
    }

    switch (type)
    {
    case DT_TYPE_AUDIO:
		mux_audio.lock();
		queue_audio.push(frame);
		mux_audio.unlock();
		break;
    case DT_TYPE_VIDEO:
		mux_video.lock();
		queue_video.push(frame);
		mux_video.unlock();
		break;
    case DT_TYPE_SUBTITLE:
		mux_sub.lock();
		queue_sub.push(frame);
		mux_sub.unlock();
		break;
    default:
		dt_warning (TAG, "[%s] unkown frame type audio or video \n", __FUNCTION__);
		return -1;
    }

    dt_debug(TAG,"AUDIO, PKT NUM:%d \n",queue_audio.size());
	dt_debug(TAG,"VIDEO, PKT NUM:%d \n",queue_video.size());
	dt_debug(TAG,"SUB, PKT NUM:%d \n",queue_sub.size());

    return 0;
}

int dtport_context::port_read_frame (dt_av_frame_t * frame, int type)
{
	dt_av_frame_t *frame_tmp;
    switch (type)
    {
    case DT_TYPE_AUDIO:
		if(queue_audio.size() <= 0)
			return -1;
		mux_audio.lock();
		frame_tmp = queue_audio.front();		    
		frame->data = frame_tmp->data;
		frame->dts = frame_tmp->dts;
		frame->duration = frame_tmp->duration;
		frame->pts = frame_tmp->pts;
		frame->size = frame_tmp->size;
		frame->type = frame_tmp->type;		
		delete(frame_tmp);
		
		queue_audio.pop();
		mux_audio.unlock();
        break;
    case DT_TYPE_VIDEO:
		if(queue_video.size() <= 0)
			return -1;
        mux_video.lock();
		frame_tmp = queue_video.front();		    
		frame->data = frame_tmp->data;
		frame->dts = frame_tmp->dts;
		frame->duration = frame_tmp->duration;
		frame->pts = frame_tmp->pts;
		frame->size = frame_tmp->size;
		frame->type = frame_tmp->type;		
		delete(frame_tmp);
		
		queue_video.pop();
		mux_video.unlock();
        break;
    case DT_TYPE_SUBTITLE:
		if(queue_sub.size() <= 0)
			return -1;
        mux_sub.lock();
		frame_tmp = queue_sub.front();		    
		frame->data = frame_tmp->data;
		frame->dts = frame_tmp->dts;
		frame->duration = frame_tmp->duration;
		frame->pts = frame_tmp->pts;
		frame->size = frame_tmp->size;
		frame->type = frame_tmp->type;		
		delete(frame_tmp);
		
		queue_sub.pop();
		mux_sub.unlock();
        break;
    default:
        dt_warning (TAG, "[%s] unkown frame type audio or video \n", __FUNCTION__);
        return -1;
    }

    //dt_info(TAG,"[%s:%d] start read frame type:%d nb:%d\n",__FUNCTION__,__LINE__,type,queue->nb_packets);
    return 0;
}

int dtport_context::port_get_state (buf_state_t * buf_state, int type)
{
    switch (type)
    {
    case DT_TYPE_AUDIO:
        buf_state->size = queue_audio.size();
        break;
    case DT_TYPE_VIDEO:
		buf_state->size = queue_video.size();
        break;
    case DT_TYPE_SUBTITLE:
		buf_state->size = queue_sub.size();
        break;
    default:
        dt_warning (TAG, "[%s] unkown frame type audio or video \n", __FUNCTION__);
        return -1;
    }
    return 0;

}

int dtport_context::port_stop ()
{
    int has_video, has_audio, has_subtitle;
    has_audio = this->param.has_audio;
    has_video = this->param.has_video;
    has_subtitle = this->param.has_subtitle;
	
	/*clear all*/
	dt_av_frame_t *frame = nullptr;
	mux_audio.lock();
	
	while(!queue_audio.empty())
	{		
		frame = queue_audio.front();
		free(frame->data);
		queue_audio.pop();
	}
	mux_audio.unlock();
	
	mux_video.lock();
	while(!queue_video.empty())
	{
		frame = queue_video.front();
		free(frame->data);
		queue_video.pop();
	}
	mux_video.unlock();
	
	mux_sub.lock();
	while(!queue_sub.empty())
	{
		frame = queue_sub.front();
		free(frame->data);
		queue_sub.pop();
	}
	mux_sub.unlock();
	
	return 0;
}
