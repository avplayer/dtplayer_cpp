#ifndef DTAUDIO_OUTPUT_H
#define DTAUDIO_OUTPUT_H

#include "dt_av.h"
#include "dtaudio_api.h"

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <thread>

#define LOG_TAG "DTAUDIO-OUTPUT"
#define PCM_WRITE_SIZE 1024

typedef struct ao_wrapper
{
    int id;
    const char *name;
	
	std::function<int (struct ao_wrapper * wrapper, void *parent)> ao_init;
	std::function<int (struct ao_wrapper * wrapper)> ao_pause;
	std::function<int (struct ao_wrapper * wrapper)> ao_resume;
	std::function<int (struct ao_wrapper * wrapper)> ao_stop;
	std::function<int (struct ao_wrapper *wrapper, uint8_t * buf, int size)> ao_write;
	std::function<int (struct ao_wrapper *wrapper)> ao_level;
	std::function<int (struct ao_wrapper *wrapper)> ao_latency;
    
    void *ao_priv;
    void *parent;

    template<typename INIT, typename PAUSE, typename RESUME, typename STOP, typename WRITE, typename LEVEL, typename LATENCY>
    ao_wrapper(int _id, const char * _name,
		INIT _init, PAUSE _pause, RESUME _resume, STOP _stop, WRITE _write, LEVEL _level, LATENCY _latency)
		  : name(_name), id(_id), ao_init(_init), ao_pause(_pause), ao_resume(_resume), ao_stop(_stop), ao_write(_write), ao_level(_level), ao_latency(_latency)
	{
	}    
} ao_wrapper_t;

#define dtao_format_t ao_id_t

typedef enum
{
    AO_STATUS_IDLE,
    AO_STATUS_PAUSE,
    AO_STATUS_RUNNING,
    AO_STATUS_EXIT,
} ao_status_t;

typedef enum _AO_CTL_ID_
{
    AO_GET_VOLUME,
    AO_ADD_VOLUME,
    AO_SUB_VOLUME,
    AO_CMD_PAUSE,
    AO_CMD_RESUME,
} ao_cmd_t;

typedef struct
{
    int aout_buf_size;
    int aout_buf_level;
} ao_state_t;

typedef struct dtaudio_output
{
    /*para */
    dtaudio_para_t para;
    ao_wrapper_t *aout_ops;
    ao_status_t status;
	std::thread audio_output_thread;
    ao_state_t state;

    uint64_t last_valid_latency;
    void *parent;               //point to dtaudio_t, can used for param of pcm get interface
    
    dtaudio_output(dtaudio_para_t &para);
	int audio_output_init (int ao_id);
	int audio_output_release ();
	int audio_output_stop ();
	int audio_output_resume ();
	int audio_output_pause ();
	int audio_output_start ();

	int audio_output_latency ();
	int64_t audio_output_get_latency ();
	int audio_output_get_level ();
    
}dtaudio_output_t;

void aout_register_all();


#endif
