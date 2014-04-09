#ifndef DT_EVENT_H
#define DT_EVENT_H

#include <thread>
#include <mutex>

typedef struct _event
{
    int type;
    union
    {
        int np;
    } para;
    int server_id;
    struct _event *next;
} event_t;

typedef struct event_server
{
    char name[1024];
    int id;
    event_t *event;
	std::mutex mux_event;
    int event_count;
    struct event_server *next;
} event_server_t;

typedef struct event_server_mgt
{
    event_server_t *server;
	std::mutex mux_server;
    int server_count;
    int exit_flag;
    std::thread transport_loop_thread;
	
} dt_server_mgt_t;

int dt_event_server_init ();
int dt_event_server_release ();
event_server_t *dt_alloc_server ();
int dt_register_server (event_server_t * server);
int dt_remove_server (event_server_t * server);
event_t *dt_alloc_event ();
int dt_send_event (event_t * event);
int dt_add_event (event_t * event, event_server_t * server);
event_t *dt_get_event (event_server_t * server);

#endif
