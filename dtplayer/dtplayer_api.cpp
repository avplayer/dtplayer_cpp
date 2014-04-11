#include "dthost_api.h"
#include "dtplayer_api.h"
#include "dtplayer.h"
#include "dt_event.h"

#define TAG "PLAYER-API"

dtplayer* open_player_module()
{
	dtplayer *player = new dtplayer;
	module_player *mod_player = new module_player;
	player->alloc_para = std::bind(&module_player::dtplayer_alloc_para,mod_player);
	player->release_para = std::bind(&module_player::dtplayer_release_para,mod_player,std::placeholders::_1);
	player->init = std::bind(&module_player::dtplayer_init,mod_player,std::placeholders::_1);
	player->start = std::bind(&module_player::dtplayer_start,mod_player);
	player->pause = std::bind(&module_player::dtplayer_pause,mod_player);
	player->resume = std::bind(&module_player::dtplayer_resume,mod_player);
	player->stop = std::bind(&module_player::dtplayer_stop,mod_player);
    player->seek = std::bind(&module_player::dtplayer_seek,mod_player,std::placeholders::_1);
	player->get_state = std::bind(&module_player::dtplayer_get_states,mod_player,std::placeholders::_1);
	
	mod_player->player_ext = player;
    dt_info(TAG,"OPEN PORT MODULE ok \n");
    return player;
}

dtplayer_para_t *module_player::dtplayer_alloc_para ()
{
    dtplayer_para_t *para = (dtplayer_para_t *) malloc (sizeof (dtplayer_para_t));
    if (!para)
    {
        dt_info (TAG, "DTPLAYER PARA ALLOC FAILED \n");
        return NULL;
    }
    para->no_audio = para->no_video = para->no_sub = -1;
    para->height = para->width = -1;
    para->loop_mode = 0;
    para->audio_index = para->video_index = para->sub_index = -1;
    para->update_cb = NULL;
    para->sync_enable = -1;
    return para;
}

int module_player::dtplayer_release_para (dtplayer_para_t * para)
{
    if (para)
        free (para);
    para = NULL;
    return 0;
}

int module_player::dtplayer_init (dtplayer_para_t * para)
{
    int ret = 0;
    if (!para)
        return -1;
    player_register_all();
	
	dtplayer_para_t &ppara = *para;
    dtp_ctx = new dtplayer_context(ppara);
    if (!dtp_ctx)
    {
        dt_error (TAG, "dtplayer context malloc failed \n");
        return -1;
    }
    
    /*init server for player and process */
    dt_event_server_init ();
    /*init player */
    ret = dtp_ctx->player_init ();
    if (ret < 0)
    {
        dt_error (TAG, "PLAYER INIT FAILED \n");
        return -1;
    }
    return 0;
}

int module_player::dtplayer_start ()
{
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_PLAYER;
    event->type = PLAYER_EVENT_START;
    dt_send_event (event);
    return 0;
}

int module_player::dtplayer_pause ()
{
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_PLAYER;
    event->type = PLAYER_EVENT_PAUSE;

    dt_send_event (event);
    return 0;
}

int module_player::dtplayer_resume ()
{
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_PLAYER;
    event->type = PLAYER_EVENT_RESUME;

    dt_send_event (event);
    return 0;
}

int module_player::dtplayer_stop ()
{
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_PLAYER;
    event->type = PLAYER_EVENT_STOP;
    dt_send_event (event);

    /*need to wait until player stop ok */
	dtp_ctx->event_loop_thread.join();
	delete(dtp_ctx);
    return 0;
}

int module_player::dtplayer_seek (int s_time)
{
    //get current time
    int64_t current_time = dtp_ctx->state.cur_time;
    int64_t full_time = dtp_ctx->media_info->duration;
    int seek_time = current_time + s_time;
    if (seek_time < 0)
        seek_time = 0;
    if (seek_time > full_time)
        seek_time = full_time;
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_PLAYER;
    event->type = PLAYER_EVENT_SEEK;
    event->para.np = seek_time;
    dt_send_event (event);

    return 0;
}

int module_player::dtplayer_get_states (player_state_t * state)
{
    memcpy (state, &dtp_ctx->state, sizeof (player_state_t));
    return 0;
}
