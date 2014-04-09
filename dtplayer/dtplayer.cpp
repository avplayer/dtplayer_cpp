#include "dtplayer.h"
#include "dtdemuxer_api.h"
#include "dtplayer_io.h"
#include "dtplayer_util.h"
#include "dt_ini.h"
#include "dtstream.h"
#include "dtdemuxer.h"
#include "dtaudio.h"
#include "dtvideo.h"

#define TAG "PLAYER"

static void *event_handle_loop (dtplayer_context_t * dtp_ctx);

void player_register_all()
{
    stream_register_all();
    demuxer_register_all();
    audio_register_all();
    video_register_all();
}

dtplayer_context::dtplayer_context(dtplayer_para_t& para)
{
	player_para.audio_index = para.audio_index;
	strcpy(player_para.file_name,para.file_name);
	player_para.height = para.height;
	player_para.loop_mode = para.loop_mode;
	player_para.no_audio = para.no_audio;
	player_para.no_sub = para.no_sub;
	player_para.no_video = para.no_video;
	player_para.sub_index = para.sub_index;
	player_para.sync_enable = para.sync_enable;
	player_para.update_cb = para.update_cb;
	player_para.video_index = para.video_index;
	player_para.width = para.width;
	
	demuxer_priv = nullptr;
	host_priv = nullptr;
	player_server = nullptr;	
}

void dtplayer_context::set_player_status (player_status_t status)
{
    this->state.last_status = this->state.cur_status;
    this->state.cur_status = status;
}

player_status_t dtplayer_context::get_player_status ()
{
    return this->state.cur_status;
}

int dtplayer_context::player_server_init ()
{
    event_server_t *server = dt_alloc_server ();
    dt_info (TAG, "PLAYER SERVER INIT:%p \n", server);
    server->id = EVENT_SERVER_PLAYER;
    strcpy (server->name, "SERVER-PLAYER");
    dt_register_server (server);
    this->player_server = (void *) server;
    return 0;
}

int dtplayer_context::player_server_release ()
{
    event_server_t *server = (event_server_t *) this->player_server;
    dt_remove_server (server);
    this->player_server = NULL;
    return 0;
}

/*port from player_update.cpp*/
int dtplayer_context::calc_cur_time (host_state_t * host_state)
{
    player_ctrl_t *ctrl_info = &this->ctrl_info;
    player_state_t *play_stat = &this->state;

    if (ctrl_info->start_time > 0)
    {
        play_stat->start_time = ctrl_info->start_time;
        dt_info (TAG, "START TIME:%lld \n", ctrl_info->start_time);
    }

    if (ctrl_info->first_time == -1 && host_state->cur_systime != -1)
    {
        ctrl_info->first_time = host_state->cur_systime;
        dt_info (TAG, "SET FIRST TIME:%lld \n", ctrl_info->first_time);
    }
    //int64_t sys_time =(host_state->cur_systime>play_stat->start_time)?(host_state->cur_systime - play_stat->start_time):host_state->cur_systime;
    int64_t sys_time = (host_state->cur_systime > ctrl_info->first_time) ? (host_state->cur_systime - ctrl_info->first_time) : host_state->cur_systime;
    play_stat->cur_time = sys_time / 90000;
    play_stat->cur_time_ms = sys_time / 90;
    return 0;
}

/*
 * handle callback, update status and cur_time to uplevel
 * */
int dtplayer_context::player_handle_cb ()
{
    player_state_t *state = &this->state;
    if (this->update_cb)
        this->update_cb (state);
    return 0;

}

void dtplayer_context::player_update_state ()
{
    player_state_t *play_stat = &this->state;
    host_state_t host_state;

    /*update host state */
	dthost *host = this->host_ext;
    host->get_state (&host_state);

    /*calc cur time */
    this->calc_cur_time (&host_state);

    /*show info */
    dt_info (TAG, "apkt_size:%d vpkt_size:%d cur_time:%lld(s) %lld(ms) duration:%lld(s) \n ", host_state.apkt_size, host_state.vpkt_size, play_stat->cur_time, play_stat->cur_time_ms, this->media_info->duration);
}

int dtplayer_context::player_init ()
{
	dtplayer_para_t *para;
	player_ctrl_t *ctrl_info;
    char value[512];
    int sync_enable_ini = -1;
    int no_audio_ini = -1;
    int no_video_ini = -1;
    int no_sub_ini = -1;

    int ret = 0;
    this->set_player_status (PLAYER_STATUS_INIT_ENTER);
    dt_info (TAG, "[%s:%d] START PLAYER INIT\n", __FUNCTION__, __LINE__);
    this->update_cb = this->player_para.update_cb;

    /* init server */
    this->player_server_init ();

    dtdemuxer_para_t demux_para;
    demux_para.file_name = this->player_para.file_name;
    ret = dtdemuxer_open (&this->demuxer_priv, &demux_para, this);
    if (ret < 0)
    {
        ret = -1;
        goto ERR1;
    }
    this->media_info = dtdemuxer_get_media_info (this->demuxer_priv);

    /* setup player ctrl info */
    para = &this->player_para;
    ctrl_info = &this->ctrl_info;
    ctrl_info->start_time = this->media_info->start_time;
    ctrl_info->first_time = -1;
    ctrl_info->has_audio = (para->no_audio == -1) ? this->media_info->has_audio : (!para->no_audio);
    ctrl_info->has_video = (para->no_video == -1) ? this->media_info->has_video : (!para->no_video);
    ctrl_info->has_sub = (para->no_sub == -1) ? this->media_info->has_sub : (!para->no_sub);

    if (!ctrl_info->has_audio && !ctrl_info->has_video)
    {
        dt_info (TAG, "HAVE NO A-V STREAM \n");
        return -1;
    }

    if (GetEnv ("PLAYER", "player.syncenable", value) > 0)
        sync_enable_ini = atoi (value);
    if (para->sync_enable != -1)
        ctrl_info->sync_enable = para->sync_enable;
    else
        ctrl_info->sync_enable = (sync_enable_ini == -1) ? (ctrl_info->has_audio && ctrl_info->has_video) : sync_enable_ini;

    ctrl_info->cur_ast_index = (para->audio_index == -1) ? this->media_info->cur_ast_index : para->audio_index;
    ctrl_info->cur_vst_index = (para->video_index == -1) ? this->media_info->cur_vst_index : para->video_index;
    ctrl_info->cur_sst_index = (para->sub_index == -1) ? this->media_info->cur_sst_index : para->sub_index;
    dt_info (TAG, "ast_idx:%d vst_idx:%d sst_idx:%d \n", ctrl_info->cur_ast_index, ctrl_info->cur_vst_index, ctrl_info->cur_sst_index);


    if (GetEnv ("PLAYER", "player.noaudio", value) > 0)
        no_audio_ini = atoi (value);
    if (GetEnv ("PLAYER", "player.novideo", value) > 0)
        no_video_ini = atoi (value);
    if (GetEnv ("PLAYER", "player.nosub", value) > 0)
        no_sub_ini = atoi (value);
    dt_info (TAG, "ini noaudio:%d novideo:%d nosub:%d \n", no_audio_ini, no_video_ini, no_sub_ini);

    if (para->no_audio != -1)
        ctrl_info->has_audio = !para->no_audio;
    else
        ctrl_info->has_audio = (no_audio_ini == -1) ? this->media_info->has_audio : !no_audio_ini;
    if (para->no_video != -1)
        ctrl_info->has_video = !para->no_video;
    else
        ctrl_info->has_video = (no_video_ini == -1) ? this->media_info->has_video : !no_video_ini;
    if (para->no_sub != -1)
        ctrl_info->has_sub = !para->no_sub;
    else
        ctrl_info->has_sub = (no_sub_ini) ? this->media_info->has_sub : !no_sub_ini;

    if (!ctrl_info->has_audio)
        ctrl_info->cur_ast_index = -1;
    if (!ctrl_info->has_video)
        ctrl_info->cur_vst_index = -1;
    if (!ctrl_info->has_sub)
        ctrl_info->cur_sst_index = -1;

    ctrl_info->has_sub = 0;     // do not support sub for now 
    this->media_info->no_audio = !ctrl_info->has_audio;
    this->media_info->no_video = !ctrl_info->has_video;
    this->media_info->no_sub = !ctrl_info->has_sub;

    dt_info (TAG, "Finally, ctrl info, audio:%d video:%d sub:%d \n", ctrl_info->has_audio, ctrl_info->has_video, ctrl_info->has_sub);
    dt_info (TAG, "Finally, no audio:%d no video:%d no sub:%d \n", this->media_info->no_audio, this->media_info->no_video, this->media_info->no_sub);

    /*dest width height */
    ctrl_info->width = para->width;
    ctrl_info->height = para->height;
	
	this->event_loop_thread = std::thread(event_handle_loop,this);
	
    dt_info (TAG, "[%s:%d] END PLAYER INIT, RET = %d\n", __FUNCTION__, __LINE__, ret);
    this->set_player_status (PLAYER_STATUS_INIT_EXIT);
    this->player_handle_cb ();
    return 0;
  ERR2:
    dtdemuxer_close (this->demuxer_priv);
  ERR1:
    this->player_server_release ();
    this->set_player_status (PLAYER_STATUS_ERROR);
    this->player_handle_cb ();
    return -1;
}

int dtplayer_context::player_start ()
{
    int ret = 0;

    this->set_player_status (PLAYER_STATUS_START);
    ret = player_host_init ();
    if (ret < 0)
    {
        dt_error (TAG, "[%s:%d] player_host_init failed!\n", __FUNCTION__, __LINE__);
        goto ERR3;
    }
    ret = start_io_thread ();
    if (ret == -1)
    {
        dt_error (TAG "file:%s [%s:%d] player io thread start failed \n", __FILE__, __FUNCTION__, __LINE__);
        goto ERR2;
    }

    ret = player_host_start ();
    if (ret != 0)
    {
        dt_error (TAG "file:%s [%s:%d] player host start failed \n", __FILE__, __FUNCTION__, __LINE__);
        this->set_player_status (PLAYER_STATUS_ERROR);
        goto ERR1;
    }
    dt_info (TAG, "PLAYER START OK\n");
    this->set_player_status (PLAYER_STATUS_RUNNING);
    this->player_handle_cb ();
    return 0;
  ERR1:
    stop_io_thread ();
  ERR2:
    player_host_stop ();
  ERR3:
    this->set_player_status (PLAYER_STATUS_ERROR);
    this->player_handle_cb ();
    return ret;
}

int dtplayer_context::player_pause ()
{
    if (this->get_player_status () == PLAYER_STATUS_PAUSED)
        return this->player_resume ();

    if (player_host_pause () == -1)
    {
        dt_error (TAG, "PAUSE NOT HANDLED\n");
        return -1;
    }
    this->set_player_status (PLAYER_STATUS_PAUSED);
    this->player_handle_cb ();
    return 0;
}

int dtplayer_context::player_resume ()
{
    if (this->get_player_status () != PLAYER_STATUS_PAUSED)
        return -1;
    if (player_host_resume () == -1)
    {
        dt_error (TAG, "RESUME NOT HANDLED\n");
        return -1;
    }
    this->set_player_status (PLAYER_STATUS_RESUME);
    this->player_handle_cb ();
    this->set_player_status (PLAYER_STATUS_RUNNING);
    return 0;
}

int dtplayer_context::player_seekto (int seek_time)
{
    if (this->get_player_status () < PLAYER_STATUS_INIT_EXIT)
        return -1;
    this->set_player_status (PLAYER_STATUS_SEEK_ENTER);
    pause_io_thread ();
    player_host_pause ();
    int ret = dtdemuxer_seekto (this->demuxer_priv, seek_time);
    if (ret == -1)
        goto FAIL;
    player_host_stop ();
    player_host_init ();
    resume_io_thread ();
    player_host_start ();
    this->set_player_status (PLAYER_STATUS_SEEK_EXIT);
    this->player_handle_cb ();
    this->set_player_status (PLAYER_STATUS_RUNNING);
    dt_info(TAG,"SEEK TO :%d OK\n",seek_time);
    return 0;
  FAIL:
    //seek fail, continue running
    resume_io_thread ();
    player_host_resume ();
    this->set_player_status (PLAYER_STATUS_RUNNING);
    return 0;
}

int dtplayer_context::player_stop ()
{
    dt_info (TAG, "PLAYER STOP STATUS SET\n");
    this->set_player_status (PLAYER_STATUS_STOP);
    return 0;
}

int dtplayer_context::player_handle_event ()
{
    event_server_t *server = (event_server_t *) this->player_server;
    event_t *event = dt_get_event (server);

    if (!event)
    {
        dt_debug (TAG, "GET EVENT NULL \n");
        return 0;
    }
    dt_info (TAG, "GET EVENT:%d \n", event->type);
    switch (event->type)
    {
    case PLAYER_EVENT_START:
        this->player_start ();
        break;
    case PLAYER_EVENT_PAUSE:
        this->player_pause ();
        break;
    case PLAYER_EVENT_RESUME:
        this->player_resume ();
        break;
    case PLAYER_EVENT_STOP:
        this->player_stop ();
        break;
    case PLAYER_EVENT_SEEK:
        this->player_seekto (event->para.np);
        break;
    default:
        break;
    }
    if (event)
    {
        free (event);
        event = NULL;
    }
    return 0;
}

static void *event_handle_loop (dtplayer_context_t * dtp_ctx)
{
    while (1)
    {
        dtp_ctx->player_handle_event ();
        if (dtp_ctx->get_player_status () == PLAYER_STATUS_STOP)
            goto QUIT;
        usleep (300 * 1000);    // 1/3s update
        if (dtp_ctx->get_player_status () != PLAYER_STATUS_RUNNING)
            continue;
        if (dtp_ctx->get_player_status () == PLAYER_STATUS_RUNNING)
            dtp_ctx->player_update_state ();
        dtp_ctx->player_handle_cb ();
        if (!dtp_ctx->ctrl_info.eof_flag)
            continue;
        if (dtp_ctx->host_ext->get_out_closed() == 1)
            goto QUIT;
    }
    /* when playend itself ,we need to release manually */
  QUIT:
    dtp_ctx->stop_io_thread ();
    dtp_ctx->player_host_stop ();
    dtdemuxer_close (dtp_ctx->demuxer_priv);
    dtp_ctx->player_server_release ();
    dt_event_server_release ();
    dt_info (TAG, "EXIT PLAYER EVENT HANDLE LOOP\n");
    dtp_ctx->set_player_status (PLAYER_STATUS_EXIT);
    dtp_ctx->player_handle_cb ();

    return NULL;
}
