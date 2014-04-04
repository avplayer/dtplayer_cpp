#include "dtport.h"

#define TAG "PORT-MGT"

dtport_context::dtport_context(dtport_para_t& para)
{
	param.has_audio = para.has_audio;
	param.has_subtitle = para.has_subtitle;
	param.has_video = para.has_video;
	
	this->status = PORT_STATUS_IDLE;
}

int dtport_context::port_stop ()
{
    int has_video, has_audio, has_subtitle;
    has_audio = this->param.has_audio;
    has_video = this->param.has_video;
    has_subtitle = this->param.has_subtitle;
    if (has_audio)
        packet_queue_release (&this->queue_audio);
    if (has_video)
        packet_queue_release (&this->queue_video);
    if (has_subtitle)
        packet_queue_release (&this->queue_subtitle);
    return 0;
}

int dtport_context::port_init ()
{
    int ret = 0;
    if (this->param.has_audio == 0 && this->param.has_video == 0)
    {
        dt_error (TAG, "[%s:%d] no av stream found \n", __FUNCTION__, __LINE__);
        return -1;
    }
    if (this->param.has_audio)
    {
        ret = packet_queue_init (&(this->queue_audio));
        if (ret < 0)
        {
            dt_error (TAG, "[%s:%d] port  init audio queeu failed has_audio:%d\n", __FUNCTION__, __LINE__, this->param.has_audio);
            goto ERR0;
        }
        dt_info (TAG, "[%s:%d] port start init audio queeu has_audio:%d\n", __FUNCTION__, __LINE__, this->param.has_audio);

    }
    if (this->param.has_video)
    {
        dt_info (TAG, "[%s:%d] port start init video queeu has_video:%d\n", __FUNCTION__, __LINE__, this->param.has_video);
        packet_queue_init (&(this->queue_video));
    }
    if (this->param.has_subtitle)
        packet_queue_init (&(this->queue_subtitle));
    return ret;
  ERR0:
    return ret;
}

int dtport_context::port_write_frame (dt_av_frame_t * frame, int type)
{
    int audio_write_enable = 1;
    int video_write_enable = 1;

    if (this->param.has_audio)
        audio_write_enable = (this->queue_audio.nb_packets < QUEUE_MAX_PACK_NUM);
    else
        audio_write_enable = 0;
    if (this->param.has_video)
        video_write_enable = (this->queue_video.nb_packets < QUEUE_MAX_PACK_NUM);
    else
        video_write_enable = 0;

    if (!audio_write_enable && !video_write_enable)
    {
        dt_debug (TAG, "A-V HAVE ENOUGH BUFFER,DO NOT WRITE\n");
        return -1;
    }

    dt_packet_queue_t *queue;
    switch (type)
    {
    case DT_TYPE_AUDIO:
        queue = &this->queue_audio;
        break;
    case DT_TYPE_VIDEO:
        queue = &this->queue_video;
        break;
    case DT_TYPE_SUBTITLE:
        queue = &this->queue_subtitle;
        break;
    default:
        dt_warning (TAG, "[%s] unkown frame type audio or video \n", __FUNCTION__);
        return -1;
    }

    dt_debug (TAG, "[%s:%d] start write frame type:%d NB_packet:%d queue->size:%d\n", __FUNCTION__, __LINE__, type, queue->nb_packets, queue->size);

    return packet_queue_put (queue, frame);
}

int dtport_context::port_read_frame (dt_av_frame_t * frame, int type)
{
    dt_packet_queue_t *queue;
    switch (type)
    {
    case DT_TYPE_AUDIO:
        queue = &this->queue_audio;
        break;
    case DT_TYPE_VIDEO:
        queue = &this->queue_video;
        break;
    case DT_TYPE_SUBTITLE:
        queue = &this->queue_subtitle;
        break;
    default:
        dt_warning (TAG, "[%s] unkown frame type audio or video \n", __FUNCTION__);
        return -1;
    }
    //dt_info(TAG,"[%s:%d] start read frame type:%d nb:%d\n",__FUNCTION__,__LINE__,type,queue->nb_packets);
    return packet_queue_get (queue, frame);
}

int dtport_context::port_get_state (buf_state_t * buf_state, int type)
{
    dt_packet_queue_t *queue;
    switch (type)
    {
    case DT_TYPE_AUDIO:
        queue = &this->queue_audio;
        break;
    case DT_TYPE_VIDEO:
        queue = &this->queue_video;
        break;
    case DT_TYPE_SUBTITLE:
        queue = &this->queue_subtitle;
        break;
    default:
        dt_warning (TAG, "[%s] unkown frame type audio or video \n", __FUNCTION__);
        return -1;
    }
    //printf("[%s:%d] start read status type:%d size:%d\n",__FUNCTION__,__LINE__,type,queue->nb_packets);
    buf_state->data_len = queue->size;
    buf_state->size = queue->nb_packets;
    return 0;

}
