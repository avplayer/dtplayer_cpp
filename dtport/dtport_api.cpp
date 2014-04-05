#include "dtport_api.h"
#include "dtport.h"

//==Part1:Control
//
int dtport_stop (void *port)
{
    int ret = 0;
    dtport_context_t *pctx = (dtport_context_t *) port;
	pctx->port_stop();
	delete(pctx);
    return ret;

}

int dtport_init (void **port, dtport_para_t * para, void *parent)
{
    int ret;	
    dtport_para_t &ppara = *para;
    dtport_context_t *pctx = new dtport_context(ppara);
    
    ret = pctx->port_init();
    if (ret < 0)
    {
        dt_error ("[%s:%d] dtport_init failed \n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR1;
    }
    *port = (void *) pctx;
    pctx->parent = parent;
    return ret;
  ERR1:
    free (pctx);
  ERR0:
    return ret;
}

//==Part2:DATA IO Relative
int dtport_write_frame (void *port, dt_av_frame_t * frame, int type)
{
    dtport_context_t *pctx = (dtport_context_t *) port;
    return pctx->port_write_frame (frame, type);
}

int dtport_read_frame (void *port, dt_av_frame_t * frame, int type)
{
    dtport_context_t *pctx = (dtport_context_t *) port;
    return pctx->port_read_frame (frame, type);
}

//==Part3:State
int dtport_get_state (void *port, buf_state_t * buf_state, int type)
{
    dtport_context_t *pctx = (dtport_context_t *) port;
    return pctx->port_get_state (buf_state, type);
}
