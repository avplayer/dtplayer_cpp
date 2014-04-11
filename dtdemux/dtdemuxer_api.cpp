#include "dtdemuxer_api.h"
#include "dtdemuxer.h"

#define TAG "DEMUX-API"

dtdemux* open_demux_module()
{
	dtdemux *demux = new dtdemux;
	module_demux *mod_demux = new module_demux;
	
	demux->open = std::bind(&module_demux::dtdemuxer_open,mod_demux,std::placeholders::_1);
	demux->close = std::bind(&module_demux::dtdemuxer_close,mod_demux);
	
	demux->read_frame = std::bind(&module_demux::dtdemuxer_read_frame,mod_demux,std::placeholders::_1);
	demux->get_media_info = std::bind(&module_demux::dtdemuxer_get_media_info,mod_demux);
	demux->seekto = std::bind(&module_demux::dtdemuxer_seekto,mod_demux,std::placeholders::_1);
    
	mod_demux->demux_ext = demux;
    dt_info(TAG,"OPEN PORT MODULE ok \n");
    return demux;
}

int module_demux::dtdemuxer_open (dtdemuxer_para_t * para)
{
	dtdemuxer_para_t &dpara = *para;
    dem_ctx = new dtdemuxer_context(dpara);
    if (dem_ctx->demuxer_open () == -1)
    {
        dt_error (TAG, "demuxer context open failed \n");
        delete(dem_ctx);
        return -1;
    }

    dt_info (TAG, "demuxer context open success \n");
    return 0;
}

dt_media_info_t *module_demux::dtdemuxer_get_media_info ()
{
    return &(dem_ctx->media_info);
}

int module_demux::dtdemuxer_read_frame (dt_av_frame_t * frame)
{
    return dem_ctx->demuxer_read_frame (frame);
}

int module_demux::dtdemuxer_seekto (int timestamp)
{
    return dem_ctx->demuxer_seekto (timestamp);
}

int module_demux::dtdemuxer_close ()
{
    dem_ctx->demuxer_close ();
	delete(dem_ctx);
    dt_info (TAG, "dtdemuxer module close ok\n");
    return 0;
}
