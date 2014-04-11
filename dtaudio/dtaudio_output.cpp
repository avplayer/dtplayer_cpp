
#include <vector>

#include "dtaudio_output.h"
#include "dtaudio.h"

#define TAG "AUDIO-OUT"
//#define DTAUDIO_DUMP_PCM 1
#define REGISTER_AO(X, x)	 	\
	{							\
		extern ao_wrapper_t ao_##x##_ops; 	\
		register_ao(ao_##x##_ops); 	\
	}
static std::vector<ao_wrapper_t> ao_wrappers;

static void register_ao (const ao_wrapper_t & ao)
{
	ao_wrappers.push_back(ao);
}

void aout_register_ext (ao_wrapper_t & ao)
{
    ao_wrappers.push_back(ao);
}

void aout_register_all ()
{
    /*Register all audio_output */
    //REGISTER_AO (NULL, null);
#ifdef ENABLE_AO_SDL
    REGISTER_AO (SDL, sdl);
#endif

#ifdef ENABLE_AO_SDL2
    REGISTER_AO (SDL2, sdl2);
#endif

#ifdef ENABLE_AO_ALSA
    REGISTER_AO (ALSA, alsa);
#endif
//    REGISTER_AO (OSS, oss);
    return;
}

/*default alsa*/
static int select_ao_device (dtaudio_output_t * ao, int id)
{
    if(id == -1) // user did not choose vo,use default one
    {
        if(ao_wrappers.empty())
            return -1;
        ao->aout_ops = & ao_wrappers[0];
        return 0;
    }

	for (std::vector< ao_wrapper_t >::iterator it = ao_wrappers.begin(); it != ao_wrappers.end(); it++)
	{
		if (it->id ==  id)
		{
			dt_info (TAG, "[%s:%d]select--%s audio device \n", __FUNCTION__, __LINE__, it->name);
			return 0;
		}
	}
    dt_info (LOG_TAG, "no valid ao device found\n");
    return -1;
}

dtaudio_output::dtaudio_output(dtaudio_para_t& _para)
{
	para.channels = _para.channels;
	para.samplerate = _para.samplerate;
	para.dst_channels = _para.dst_channels;
	para.dst_samplerate = _para.dst_samplerate;
	para.data_width = _para.data_width;
	para.bps = _para.bps;
	para.afmt = _para.afmt;
	
	para.den = _para.den;
	para.num = _para.num;
	
	para.extradata_size = _para.extradata_size;
	if(_para.extradata_size > 0)
	{
		for(int i = 0; i < _para.extradata_size; i++)
			para.extradata[i] = _para.extradata[i];
	}
	para.audio_filter = _para.audio_filter;
	para.audio_output = _para.audio_output;
	para.avctx_priv = _para.avctx_priv;
	
	this->status = AO_STATUS_IDLE;
}


int dtaudio_output::audio_output_start ()
{
    /*start playback */
    this->status = AO_STATUS_RUNNING;
    return 0;
}

int dtaudio_output::audio_output_pause ()
{
    this->status = AO_STATUS_PAUSE;
    ao_wrapper_t *wrapper = this->aout_ops;
    wrapper->ao_pause (wrapper);
    return 0;
}

int dtaudio_output::audio_output_resume ()
{
    this->status = AO_STATUS_RUNNING;
    ao_wrapper_t *wrapper = this->aout_ops;
    wrapper->ao_resume (wrapper);
    return 0;
}

int dtaudio_output::audio_output_stop ()
{
    this->status = AO_STATUS_EXIT;
    this->audio_output_thread.join();
    ao_wrapper_t *wrapper = this->aout_ops;
    wrapper->ao_stop (wrapper);
    dt_info (TAG, "[%s:%d] aout stop ok \n", __FUNCTION__, __LINE__);
    return 0;
}

int dtaudio_output::audio_output_latency ()
{
    if (this->status == AO_STATUS_IDLE)
        return 0;
    if (this->status == AO_STATUS_PAUSE)
        return this->last_valid_latency;
    ao_wrapper_t *wrapper = this->aout_ops;
    this->last_valid_latency = wrapper->ao_latency (wrapper);
    return this->last_valid_latency;
}

int dtaudio_output::audio_output_get_level ()
{

    ao_wrapper_t *wrapper = this->aout_ops;
    return wrapper->ao_level(wrapper);
}

static void *audio_output_loop (void *args)
{
    dtaudio_output_t *ao = (dtaudio_output_t *) args;
    ao_wrapper_t *wrapper = ao->aout_ops;
    
    uint8_t buffer[PCM_WRITE_SIZE];
    int rlen, ret, wlen;
    rlen = ret = wlen = 0;
   
    for (;;)
    {
        if (ao->status == AO_STATUS_EXIT)
            goto EXIT;
        if (ao->status == AO_STATUS_IDLE || ao->status == AO_STATUS_PAUSE)
        {
            usleep (1000);
            continue;
        }

        /* update audio pts */
        ao->parent->audio_update_pts ();

        /*read data from filter or decode buffer */
        if (rlen <= 0)
        {
            rlen = ao->parent->audio_output_read (buffer, PCM_WRITE_SIZE);
            if (rlen <= 0)
            {
                dt_debug (LOG_TAG, "pcm read failed! \n");
                usleep (1000);
                continue;
            }
#ifdef DTAUDIO_DUMP_PCM
            FILE *fp = fopen ("pcm.out", "ab+");
            int flen = 0;
            if (fp)
            {
                flen = fwrite (buffer, 1, rlen, fp);
                if (flen <= 0)
                    dt_info (LOG_TAG, "pcm dump failed! \n");
                fclose (fp);
            }
            else
                dt_error (TAG, "pcm out open failed! \n");
#endif
        }
        /*write to ao device */
        wlen = wrapper->ao_write (wrapper, buffer, rlen);
        if (wlen <= 0)
        {
            usleep (1000);
            continue;
        }

        rlen -= wlen;
        if (rlen > 0)
            memmove (buffer, buffer + wlen, rlen);
        wlen = 0;
    }
  EXIT:
    dt_info (LOG_TAG, "[%s:%d]ao playback thread exit\n", __FUNCTION__, __LINE__);
    return NULL;

}

int dtaudio_output::audio_output_init (int ao_id)
{
    int ret = 0;
    
    /*select ao device */
    ret = select_ao_device (this, ao_id);
    if (ret < 0)
        return -1;
    
    ao_wrapper_t *wrapper = this->aout_ops;
    wrapper->ao_init (wrapper,this);
    dt_info (TAG, "[%s:%d] audio output init success\n", __FUNCTION__, __LINE__);
    
    this->audio_output_thread = std::thread(audio_output_loop,this);
	
    dt_info (TAG, "[%s:%d] create audio output thread success\n", __FUNCTION__, __LINE__);
    return ret;
}

int64_t dtaudio_output::audio_output_get_latency ()
{
    if (this->status == AO_STATUS_IDLE)
        return 0;
    if (this->status == AO_STATUS_PAUSE)
        return this->last_valid_latency;
    ao_wrapper_t *wrapper = this->aout_ops;
    this->last_valid_latency = wrapper->ao_latency (wrapper);
    return this->last_valid_latency;
}
