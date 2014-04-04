#include "dtvideo.h"

#define TAG "VIDEO-API"

//==Part1:Control
int dtvideo_init (void **video_priv, dtvideo_para_t * para, void *parent)
{
    int ret = 0;
	dtvideo_para_t &vpara = *para;   
    dtvideo_context_t *vctx = new dtvideo_context(vpara);	
    vctx->parent = parent;	
    ret = vctx->video_init ();
    if (ret < 0)
    {
        dt_error (TAG, "[%s:%d] dtvideo_mgt_init failed \n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR1;
    }
    *video_priv = (void *) vctx;
    return ret;
  ERR1:
    delete(vctx);
  ERR0:
    return ret;
}

int dtvideo_start (void *video_priv)
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
	return vctx->video_start();
}

int dtvideo_pause (void *video_priv)
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
	return vctx->video_pause();
}

int dtvideo_resume (void *video_priv)
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
	return vctx->video_resume();

}

int dtvideo_stop (void *video_priv)
{
    int ret;
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
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
    video_priv = NULL;
    return ret;
  ERR0:
    return ret;

}

int64_t dtvideo_get_first_pts (void *video_priv)
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
	return vctx->video_get_first_pts();
}

int dtvideo_drop (void *video_priv, int64_t target_pts)
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
    return vctx->video_drop (target_pts);
}

int dtvideo_get_state (void *video_priv, dec_state_t * dec_state)
{
    int ret;
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
    ret = vctx->video_get_dec_state (dec_state);
    return ret;
}

int dtvideo_get_out_closed (void *video_priv)
{
    int ret;
    dtvideo_context_t *vctx = (dtvideo_context_t *) video_priv;
    ret = vctx->video_get_out_closed ();
    return ret;
}
