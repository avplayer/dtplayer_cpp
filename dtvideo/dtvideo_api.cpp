#include "dtvideo.h"

#define TAG "VIDEO-API"

dtvideo* open_video_module()
{
	dtvideo *video = new dtvideo;
	module_video *mod_video = new module_video;
	
	video->drop = std::bind(&module_video::dtvideo_drop,mod_video, std::placeholders::_1);
	video->init = std::bind(&module_video::dtvideo_init,mod_video,std::placeholders::_1,std::placeholders::_2);
	video->get_first_pts = std::bind(&module_video::dtvideo_get_first_pts,mod_video);
	video->get_out_closed = std::bind(&module_video::dtvideo_get_out_closed,mod_video);
	video->get_state = std::bind(&module_video::dtvideo_get_state,mod_video,std::placeholders::_1);
	video->start = std::bind(&module_video::dtvideo_start,mod_video);
	video->pause = std::bind(&module_video::dtvideo_pause,mod_video);
	video->resume = std::bind(&module_video::dtvideo_resume,mod_video);
	video->stop = std::bind(&module_video::dtvideo_stop,mod_video);
    
	mod_video->video_ext = video;
    dt_info(TAG,"OPEN VIDEO MODULE ok \n");
    return video;
}


//==Part1:Control
int module_video::dtvideo_init (dtvideo_para_t * para, dthost *host)
{
    int ret = 0;
	dtvideo_para_t &vpara = *para;   
    vctx = new dtvideo_context(vpara);
	host_ext = host;
	vctx->parent = this;
    ret = vctx->video_init ();
    if (ret < 0)
    {
        dt_error (TAG, "[%s:%d] dtvideo_mgt_init failed \n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR1;
    }
   
    return ret;
  ERR1:
    delete(vctx);
  ERR0:
    return ret;
}

int module_video::dtvideo_start ()
{
	return vctx->video_start();
}

int module_video::dtvideo_pause ()
{
	return vctx->video_pause();
}

int module_video::dtvideo_resume ()
{
	return vctx->video_resume();

}

int module_video::dtvideo_stop ()
{
    int ret;
    if (!vctx)
    {
        dt_error (TAG, "[%s:%d] dt video context == NULL\n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR0;
    }
    ret = vctx->video_stop ();
    if (ret < 0)
    {
        dt_error (TAG, "[%s:%d] DTVIDEO STOP FAILED\n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR0;
    }
    delete (vctx);
    return ret;
  ERR0:
    return ret;

}

int64_t module_video::dtvideo_get_first_pts ()
{
	return vctx->video_get_first_pts();
}

int module_video::dtvideo_drop (int64_t target_pts)
{
    return vctx->video_drop (target_pts);
}

int module_video::dtvideo_get_state (dec_state_t * dec_state)
{
    return vctx->video_get_dec_state (dec_state);
}

int module_video::dtvideo_get_out_closed ()
{
    return vctx->video_get_out_closed ();
}
