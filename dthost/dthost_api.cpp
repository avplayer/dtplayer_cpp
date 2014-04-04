#include "dthost_api.h"
#include "dthost.h"

#define TAG "HOST-EXT"

int dthost_start (void *host_priv)
{
    int ret = 0;
    dthost_context_t *hctx = (dthost_context_t *) host_priv;
    dt_debug (TAG, "hctx :%p \n", hctx);
    ret = hctx->host_start ();
    return ret;
}

int dthost_pause (void *host_priv)
{
    int ret = 0;
    dthost_context_t *hctx = (dthost_context_t *) host_priv;
    ret = hctx->host_pause ();
    return ret;
}

int dthost_resume (void *host_priv)
{
    int ret = 0;
    dthost_context_t *hctx = (dthost_context_t *) host_priv;
    ret = hctx->host_resume ();
    return ret;
}

int dthost_stop (void *host_priv)
{
    int ret = 0;
    if (!host_priv)
        return -1;
    dthost_context_t *hctx = (dthost_context_t *) host_priv;
    ret = hctx->host_stop ();
    if (ret < 0)
        goto FAIL;
    delete(hctx);
    host_priv = NULL;
  FAIL:
    return ret;

}

int dthost_init (void **host_priv, dthost_para_t * para)
{
    int ret = 0;
   
    dthost_para_t &hpara = *para;
    dthost_context_t *hctx = new dthost_context(hpara);
    dt_debug (TAG, "hctx :%p \n", hctx);
	
    ret = hctx->host_init();
    if (ret < 0)
    {
        dt_error (TAG, "[%s:%d] dthost_init failed\n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR1;
    }
    *host_priv = (void *) hctx;
    return ret;
  ERR1:
    free (hctx);
  ERR0:
    return ret;
}

//==Part2:Data IO Relative

int dthost_read_frame (void *host_priv, dt_av_frame_t * frame, int type)
{
    int ret = 0;
    if (!host_priv)
    {
        dt_error (TAG, "[%s:%d] host_priv is Null\n", __FUNCTION__, __LINE__);
        return -1;
    }
    dthost_context_t *hctx = (dthost_context_t *) host_priv;
    ret = hctx->host_read_frame (frame, type);
    return ret;
}

int dthost_write_frame (void *host_priv, dt_av_frame_t * frame, int type)
{
    int ret = 0;
    if (!host_priv)
    {
        dt_error (TAG, "[%s:%d] host_priv==NULL \n", __FUNCTION__, __LINE__);
        return -1;
    }
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    ret = hctx->host_write_frame (frame, type);
    return ret;
}

//==Part3: PTS Relative

int64_t dthost_get_apts (void *host_priv)
{
    if (!host_priv)
        return -1;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    return hctx->host_get_apts ();
}

int64_t dthost_update_apts (void *host_priv, int64_t pts)
{
    if (!host_priv)
        return -1;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    return hctx->host_update_apts (pts);
}

int64_t dthost_get_vpts (void *host_priv)
{
    if (!host_priv)
        return -1;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    return hctx->host_get_vpts ();
}

void dthost_update_vpts (void *host_priv, int64_t vpts)
{
    if (!host_priv)
        return;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    hctx->host_update_vpts (vpts);
    return;
}

int dthost_get_avdiff (void *host_priv)
{
    if (!host_priv)
        return 0;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    return hctx->host_get_avdiff ();
}

int64_t dthost_get_current_time (void *host_priv)
{
    if (!host_priv)
        return -1;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    return hctx->host_get_current_time ();
}

int64_t dthost_get_systime (void *host_priv)
{
    if (!host_priv)
        return -1;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    return hctx->host_get_systime ();
}

void dthost_update_systime (void *host_priv, int64_t systime)
{
    if (!host_priv)
        return;
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    hctx->host_update_systime (systime);
    return;
}

//==Part4:Status Relative

int dthost_get_state (void *host_priv, host_state_t * state)
{
    dthost_context_t *hctx = (dthost_context_t *) (host_priv);
    if (!host_priv)
        return -1;
    hctx->host_get_state (state);
    return 0;
}

int dthost_get_out_closed (void *host_priv)
{
    int ret = 0;
    if (!host_priv)
    {
        dt_error (TAG, "host PRIV IS NULL \n");
        return -1;
    }
    dthost_context_t *hctx = (dthost_context_t *) host_priv;
    ret = hctx->host_get_out_closed ();
    return ret;
}
