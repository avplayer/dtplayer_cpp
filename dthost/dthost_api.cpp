#include "dthost_api.h"
#include "dthost.h"

#include <functional>

#define TAG "HOST-API"

module_host::module_host()
{
	hctx = nullptr;
}

dthost * open_host_module()
{
	dthost *host = new dthost;
	module_host *mod_host = new module_host;
	
	host->init = std::bind(&module_host::dthost_init,mod_host,std::placeholders::_1);
	host->start = std::bind(&module_host::dthost_start,mod_host);
	host->pause = std::bind(&module_host::dthost_pause,mod_host);
	host->resume = std::bind(&module_host::dthost_resume,mod_host);
	host->stop = std::bind(&module_host::dthost_stop,mod_host);
	
	host->read_frame = std::bind(&module_host::dthost_read_frame,mod_host,std::placeholders::_1,std::placeholders::_2);
	host->write_frame = std::bind(&module_host::dthost_write_frame,mod_host,std::placeholders::_1,std::placeholders::_2);
	host->get_apts = std::bind(&module_host::dthost_get_apts,mod_host);
	host->update_apts = std::bind(&module_host::dthost_update_apts,mod_host,std::placeholders::_1);
	host->get_vpts = std::bind(&module_host::dthost_get_vpts,mod_host);
	host->update_vpts = std::bind(&module_host::dthost_update_vpts,mod_host,std::placeholders::_1);
	host->get_systime = std::bind(&module_host::dthost_get_systime,mod_host);
	host->update_systime = std::bind(&module_host::dthost_update_systime,mod_host,std::placeholders::_1);
	host->get_avdiff = std::bind(&module_host::dthost_get_avdiff,mod_host);
	
	host->get_current_time = std::bind(&module_host::dthost_get_current_time,mod_host);
	host->get_state = std::bind(&module_host::dthost_get_state,mod_host,std::placeholders::_1);
	host->get_out_closed = std::bind(&module_host::dthost_get_out_closed,mod_host);
    
	mod_host->host_ext = host;
    dt_info(TAG,"OPEN HOST MODULE ok \n");
    return host;
}


int module_host::dthost_start ()
{
    return hctx->host_start ();
}

int module_host::dthost_pause ()
{
    return hctx->host_pause ();
}

int module_host::dthost_resume ()
{
    return hctx->host_resume ();
}

int module_host::dthost_stop ()
{
	return hctx->host_stop ();
}

int module_host::dthost_init (dthost_para_t * para)
{
    int ret = 0;
   
    dthost_para_t &hpara = *para;
    hctx = new dthost_context(hpara);
	hctx->parent = this;
    ret = hctx->host_init();
    if (ret < 0)
    {
        dt_error (TAG, "[%s:%d] dthost_init failed\n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR1;
    }

    return ret;
  ERR1:
    delete (hctx);
    return ret;
}

int module_host::dthost_read_frame (dt_av_frame_t * frame, int type)
{
    return hctx->host_read_frame (frame, type);
}

int module_host::dthost_write_frame (dt_av_frame_t * frame, int type)
{
    return hctx->host_write_frame (frame, type);
}

int64_t module_host::dthost_get_apts ()
{
    return hctx->host_get_apts ();
}

int module_host::dthost_update_apts (int64_t pts)
{
    return hctx->host_update_apts (pts);
}

int64_t module_host::dthost_get_vpts ()
{
    return hctx->host_get_vpts ();
}

int module_host::dthost_update_vpts (int64_t vpts)
{
    return hctx->host_update_vpts (vpts);
}

int64_t module_host::dthost_get_avdiff ()
{
    return hctx->host_get_avdiff ();
}

int64_t module_host::dthost_get_current_time ()
{
    return hctx->host_get_current_time ();
}

int64_t module_host::dthost_get_systime ()
{
    return hctx->host_get_systime ();
}

int module_host::dthost_update_systime (int64_t systime)
{
    return hctx->host_update_systime (systime);
}

int module_host::dthost_get_state (host_state_t * state)
{
    return hctx->host_get_state (state);
}

int module_host::dthost_get_out_closed ()
{
    return hctx->host_get_out_closed ();
}
