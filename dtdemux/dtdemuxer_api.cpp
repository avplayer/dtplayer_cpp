#include "dtdemuxer_api.h"
#include "dtdemuxer.h"

#define TAG "DEMUX-API"
int dtdemuxer_open (void **priv, dtdemuxer_para_t * para, void *parent)
{
	dtdemuxer_para_t &dpara = *para;
    dtdemuxer_context_t *dem_ctx = new dtdemuxer_context(dpara);
    if (dem_ctx->demuxer_open () == -1)
    {
        dt_error (TAG, "demuxer context open failed \n");
        delete(dem_ctx);
        return -1;
    }

    *priv = (void *) dem_ctx;
    dem_ctx->parent = parent;
    dt_info (TAG, "demuxer context open success \n");
    return 0;
}

dt_media_info_t *dtdemuxer_get_media_info (void *priv)
{
    dtdemuxer_context_t *dem_ctx = (dtdemuxer_context_t *) priv;
    return &(dem_ctx->media_info);
}

int dtdemuxer_read_frame (void *priv, dt_av_frame_t * frame)
{
    dtdemuxer_context_t *dem_ctx = (dtdemuxer_context_t *) priv;
    return dem_ctx->demuxer_read_frame (frame);
}

int dtdemuxer_seekto (void *priv, int timestamp)
{
    dtdemuxer_context_t *dem_ctx = (dtdemuxer_context_t *) priv;
    return dem_ctx->demuxer_seekto (timestamp);
}

int dtdemuxer_close (void *priv)
{
    dtdemuxer_context_t *dem_ctx = (dtdemuxer_context_t *) priv;
    dem_ctx->demuxer_close ();
	delete(dem_ctx);
	priv = nullptr;
    dt_info (TAG, "dtdemuxer module close ok\n");
    return 0;
}
