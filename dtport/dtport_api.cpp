#include "dtport_api.h"
#include "dtport.h"

#define TAG "PORT_API"

dtport* open_port_module()
{
	dtport *port = new dtport;
	module_port *mod_port = new module_port;
	
	port->init = std::bind(&module_port::dtport_init,mod_port,std::placeholders::_1);
	port->stop = std::bind(&module_port::dtport_stop,mod_port);
	
	port->read_frame = std::bind(&module_port::dtport_read_frame,mod_port,std::placeholders::_1,std::placeholders::_2);
	port->write_frame = std::bind(&module_port::dtport_write_frame,mod_port,std::placeholders::_1,std::placeholders::_2);
	port->get_state = std::bind(&module_port::dtport_get_state,mod_port,std::placeholders::_1,std::placeholders::_2);
    
	mod_port->port_ext = port;
    dt_info(TAG,"OPEN PORT MODULE ok \n");
    return port;
}

int module_port::dtport_stop()
{
	pctx->port_stop();
	delete(pctx);
    return 0;

}

int module_port::dtport_init (dtport_para_t * para)
{
    int ret;	
    dtport_para_t &ppara = *para;
    pctx = new dtport_context(ppara);
    pctx->parent = this;
    ret = pctx->port_init();
    if (ret < 0)
    {
        dt_error ("[%s:%d] dtport_init failed \n", __FUNCTION__, __LINE__);
        ret = -1;
        goto ERR1;
    }
    return ret;
  ERR1:
    free (pctx);
  ERR0:
    return ret;
}

int module_port::dtport_write_frame (dt_av_frame_t * frame, int type)
{
    return pctx->port_write_frame (frame, type);
}

int module_port::dtport_read_frame (dt_av_frame_t * frame, int type)
{
    return pctx->port_read_frame (frame, type);
}

int module_port::dtport_get_state (buf_state_t * buf_state, int type)
{
    return pctx->port_get_state (buf_state, type);
}
