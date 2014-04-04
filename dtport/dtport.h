#ifndef DTPORT_H
#define DTPORT_H

#include "dt_packet_queue.h"
#include "dt_av.h"
#include "dtport_api.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum
{
    PORT_STATUS_IDLE = -1,
    PORT_STATUS_INITING,
    PORT_STATUS_INITED,
    PORT_STATUS_RUNNING,
    PORT_STATUS_EXIT,
} port_status_t;

typedef struct dtport_context
{
    dt_packet_queue_t queue_audio;
    dt_packet_queue_t queue_video;
    dt_packet_queue_t queue_subtitle;

    buf_state_t dps_audio;
    buf_state_t dps_video;
    buf_state_t dps_sub;

    dtport_para_t param;
    port_status_t status;
    void *parent;
	
	dtport_context(dtport_para_t &para);
	int port_stop ();
	int port_init ();
	int port_write_frame (dt_av_frame_t * frame, int type);
	int port_read_frame (dt_av_frame_t * frame, int type);
	int port_get_state (buf_state_t * buf_state, int type);
	
} dtport_context_t;

#endif
