#include "dtaudio_api.h"
#include "dtaudio.h"

#define TAG "AUDIO-API"



class module_audio
{
	dtaudio_context_t *actx;
	dthost *host;
public:
	module_audio();
	int dtaudio_init (dtaudio_para_t * para, dthost *host);
	int dtaudio_start ();
	int dtaudio_pause ();
	int dtaudio_resume ();
	int dtaudio_stop ();
	int64_t dtaudio_get_pts ();
	int dtaudio_drop (module_audio *mod,int64_t target_pts);
	int64_t dtaudio_get_first_pts ();
	int dtaudio_get_state (dec_state_t * dec_state);
	int dtaudio_get_out_closed ();
};

dtaudio* open_audio_module()
{
	dtaudio *audio = new dtaudio;
	module_audio *mod_audio = new module_audio;
	
	audio->drop = std::bind(&module_audio::dtaudio_drop,mod_audio, _1);

#if 0
	audio->get_first_pts = std::bind(&mod_audio->dtaudio_get_first_pts);
	audio->get_out_closed = std::bind(&mod_audio->dtaudio_get_out_closed);
	audio->get_pts = std::bind(&mod_audio->dtaudio_get_pts);
	audio->get_state = std::bind(&mod_audio->dtaudio_get_state,_1);
	audio->init = std::bind(&mod_audio->dtaudio_init,_1,_2);
	audio->pause = std::bind(&mod_audio->dtaudio_pause);
	audio->resume = std::bind(&mod_audio->dtaudio_resume);
	audio->stop = std::bind(&mod_audio->dtaudio_stop);
#endif

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
	host = nullptr;
}

int module_audio::dtaudio_init (dtaudio_para_t * para, dthost *_host)
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
	host = _host;
    ret = actx->audio_init();
    if (ret < 0)
    {
        dt_error ("[%s:%d] audio_init failed \n", __FUNCTION__, __LINE__);
        return ret;
    }

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
