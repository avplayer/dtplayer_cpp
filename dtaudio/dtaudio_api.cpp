#include "dtaudio_api.h"
#include "dtaudio.h"

#define TAG "AUDIO-API"

dtaudio* open_audio_module()
{
	dtaudio *audio = new dtaudio;
	module_audio *mod_audio = new module_audio;
	
	audio->drop = std::bind(&module_audio::dtaudio_drop,mod_audio, std::placeholders::_1);
	audio->init = std::bind(&module_audio::dtaudio_init,mod_audio,std::placeholders::_1,std::placeholders::_2);
	audio->get_first_pts = std::bind(&module_audio::dtaudio_get_first_pts,mod_audio);
	audio->get_out_closed = std::bind(&module_audio::dtaudio_get_out_closed,mod_audio);
	audio->get_pts = std::bind(&module_audio::dtaudio_get_pts,mod_audio);
	audio->get_state = std::bind(&module_audio::dtaudio_get_state,mod_audio,std::placeholders::_1);
	audio->start = std::bind(&module_audio::dtaudio_start,mod_audio);
	audio->pause = std::bind(&module_audio::dtaudio_pause,mod_audio);
	audio->resume = std::bind(&module_audio::dtaudio_resume,mod_audio);
	audio->stop = std::bind(&module_audio::dtaudio_stop,mod_audio);
    
	mod_audio->audio_ext = audio;
    dt_info(TAG,"OPEN AUDIO MODULE ok \n");
    return audio;
}


static int audio_server_init (dtaudio_context_t * actx)
{
    event_server_t *server = dt_alloc_server ();
    dt_info (TAG, "AUDIO SERVER INIT :%p \n", server);
    if (!server)
    {
        dt_error (TAG, "AUDIO SERVER ALLOC FAILED \n");
        return -1;
    }
    server->id = EVENT_SERVER_AUDIO;
    strcpy (server->name, "SERVER-AUDIO");
    dt_register_server (server);
    actx->audio_server = (void *) server;
    return 0;
}

static int audio_server_release (dtaudio_context_t * actx)
{
    event_server_t *server = (event_server_t *) actx->audio_server;
    dt_remove_server (server);
    actx->audio_server = NULL;
    return 0;
}

module_audio::module_audio()
{
	actx = nullptr;
	audio_ext = nullptr;
	host_ext = nullptr;
}

int module_audio::dtaudio_init (dtaudio_para_t * para, dthost *host)
{
    int ret = 0;
	
	dtaudio_para_t &apara = *para;
	actx = new dtaudio_context(apara);

    /*init server */
    ret = audio_server_init (actx);
    if (ret < 0)
    {
        dt_error (TAG, "DTAUDIO INIT FAILED \n");
        return ret;
    }
    //we need to set parent early, Since enter audio decoder loop first,will crash for parent invalid
	host_ext = host;
	actx->parent = this;
    ret = actx->audio_init();
    if (ret < 0)
    {
        dt_error ("[%s:%d] audio_init failed \n", __FUNCTION__, __LINE__);
        return ret;
    }

    
    dt_info(TAG,"DTAUDIO INIT OK\n");
    return ret;
}

int module_audio::dtaudio_start ()
{
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_AUDIO;
    event->type = AUDIO_EVENT_START;
    dt_send_event (event);
    return 0;
}

int module_audio::dtaudio_pause ()
{
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_AUDIO;
    event->type = AUDIO_EVENT_PAUSE;
    dt_send_event (event);
    return 0;
}

int module_audio::dtaudio_resume ()
{
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_AUDIO;
    event->type = AUDIO_EVENT_RESUME;
    dt_send_event (event);
    return 0;
}

int module_audio::dtaudio_stop ()
{
    int ret = 0;
    event_t *event = dt_alloc_event ();
    event->next = NULL;
    event->server_id = EVENT_SERVER_AUDIO;
    event->type = AUDIO_EVENT_STOP;
    dt_send_event (event);

    actx->event_loop_thread.join();

    audio_server_release (actx);
    delete(actx);
    return ret;
}

int64_t module_audio::dtaudio_get_pts ()
{
	return actx->audio_get_first_pts();
}

int module_audio::dtaudio_drop (int64_t target_pts)
{
    return actx->audio_drop(target_pts);
}

int64_t module_audio::dtaudio_get_first_pts ()
{
	return actx->audio_get_first_pts();
}

int module_audio::dtaudio_get_state (dec_state_t * dec_state)
{
    int ret;
	ret = actx->audio_get_dec_state(dec_state);
    return ret;
}

int module_audio::dtaudio_get_out_closed ()
{
	return actx->audio_get_out_closed();
}
