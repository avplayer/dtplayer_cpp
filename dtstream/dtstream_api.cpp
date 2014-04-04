#include "dtstream_api.h"
#include "dtstream.h"

#define TAG "STREAM_API"

int dtstream_open (void **priv, dtstream_para_t * para, void *parent)
{
	dtstream_para_t &spara = *para;
    dtstream_context_t *ctx = new dtstream_context(spara);
    if(ctx->stream_open() == -1)
    {
        dt_error(TAG,"STREAM CONTEXT OPEN FAILED \n");
        free(ctx);
        *priv = NULL;
        return -1;
    }
    *priv = (void *)ctx;
    ctx->parent = parent;
    dt_info(TAG,"STREAM CTX OPEN SUCCESS\n");
    return 0;
}

int64_t dtstream_get_size(void *priv)
{
    dtstream_context_t *stm_ctx = (dtstream_context_t *) priv;
    return stm_ctx->stream_get_size();
}

int dtstream_eof (void *priv)
{
    dtstream_context_t *stm_ctx = (dtstream_context_t *) priv;
    return stm_ctx->stream_eof();
}

int64_t dtstream_tell (void *priv)
{
    dtstream_context_t *stm_ctx = (dtstream_context_t *) priv;
    return stm_ctx->stream_tell();
}

/*
 * skip size byte 
 * maybe negitive , then seek forward
 *
 * */
int dtstream_skip (void *priv, int64_t size)
{
    dtstream_context_t *stm_ctx = (dtstream_context_t *) priv;
    stm_ctx->stream_seek(size,SEEK_CUR);
    return 1;
}

int dtstream_read (void *priv, uint8_t *buf,int len)
{
    dtstream_context_t *stm_ctx = (dtstream_context_t *) priv;
    return stm_ctx->stream_read(buf,len);
}

int dtstream_seek (void *priv, int64_t pos ,int whence)
{
    dtstream_context_t *stm_ctx = (dtstream_context_t *) priv;
    return stm_ctx->stream_seek(pos,whence);
}

int dtstream_close (void *priv)
{
    dtstream_context_t *stm_ctx = (dtstream_context_t *) priv;
    if(stm_ctx)
    {
        stm_ctx->stream_close();
        delete(stm_ctx);
    }
    priv = NULL;
    return 0;
}
