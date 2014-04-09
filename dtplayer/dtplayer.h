#ifndef DTPLAYER_H
#define DTPLAYER_H

#include "dtplayer_api.h"
#include "dt_media_info.h"
#include "dthost_api.h"
#include "dt_av.h"
#include "dt_event.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <thread>

typedef struct
{
    int status;                 // 0 start 1 pause 2 quit
    int flag;
    std::thread io_loop_thread;
} io_loop_t;

typedef struct
{
    /*stream info */
    int64_t start_time;
    int64_t first_time;

      /**/ int cur_ast_index;
    int cur_vst_index;
    int cur_sst_index;

    /*dest width height */
    int width;
    int height;
    /*ctrl flag */
    int eof_flag;
    int sync_enable;
    int has_audio;
    int has_video;
    int has_sub;

} player_ctrl_t;

typedef struct dtplayer_context
{
    dtplayer_para_t player_para;

    void *demuxer_priv;
    dt_media_info_t *media_info;

    player_ctrl_t ctrl_info;
    dthost_para_t host_para;
    void *host_priv;
	dthost *host_ext;

    player_state_t state;
    int (*update_cb) (player_state_t * state); // update player info to uplevel

    io_loop_t io_loop;
	std::thread event_loop_thread;

    void *player_server;
	
	dtplayer_context(dtplayer_para_t &para);
	void set_player_status (player_status_t status);
	player_status_t get_player_status ();
	int player_server_init ();
	int player_server_release ();
	int player_handle_event ();
	
	/*update*/
	int calc_cur_time (host_state_t * host_state);
	int player_handle_cb ();
	void player_update_state ();
	
	/*io*/
	int start_io_thread ();
	int pause_io_thread ();
	int resume_io_thread ();
	int stop_io_thread ();
	
	/*utils*/
	int player_host_init ();
	int player_host_start ();
	int player_host_pause ();
	int player_host_resume ();
	int player_host_stop ();
	
	int player_init ();
	int player_start ();
	int player_pause ();
	int player_resume ();
	int player_seekto (int seek_time);
	int player_stop ();
	
} dtplayer_context_t;

void player_register_all();

#endif
