#ifndef DTPLAYER_API_H
#define DTPLAYER_API_H

#include "dt_av.h"

typedef enum
{
    PLAYER_STATUS_INVALID = -1,
    PLAYER_STATUS_IDLE,

    PLAYER_STATUS_INIT_ENTER,
    PLAYER_STATUS_INIT_EXIT,

    PLAYER_STATUS_START,
    PLAYER_STATUS_RUNNING,

    PLAYER_STATUS_PAUSED,
    PLAYER_STATUS_RESUME,
    PLAYER_STATUS_SEEK_ENTER,
    PLAYER_STATUS_SEEK_EXIT,

    PLAYER_STATUS_ERROR,
    PLAYER_STATUS_STOP,
    PLAYER_STATUS_PLAYEND,
    PLAYER_STATUS_EXIT,
} player_status_t;

typedef struct
{
    /* player state */
    player_status_t cur_status;
    player_status_t last_status;

    int64_t cur_time_ms;
    int64_t cur_time;

    int64_t start_time;
} player_state_t;

typedef struct dtplayer_para
{
    char file_name[FILE_NAME_MAX_LENGTH];
    int video_index;
    int audio_index;
    int sub_index;

    int loop_mode;
    int no_audio;
    int no_video;
    int no_sub;
    int sync_enable;

    int width;
    int height;

    int (*update_cb) (player_state_t * sta);
} dtplayer_para_t;

class dtplayer
{
public:
	dtplayer(){};
	std::function<dtplayer_para_t *()>alloc_para;
	std::function<int (dtplayer_para_t * para)>release_para;
	
	std::function<int (dtplayer_para_t * para)>init;
	std::function<int ()>start;
	std::function<int ()>pause;
	std::function<int ()>resume;
	std::function<int ()>stop;
	std::function<int (int s_time)>seek;
	std::function<int (player_state_t * state)>get_state;
};

struct dtplayer_context;

class module_player
{
public:
	dtplayer *player_ext;
	struct dtplayer_context *dtp_ctx;

	dtplayer_para_t *dtplayer_alloc_para ();
	int dtplayer_release_para (dtplayer_para_t * para);
	
	int dtplayer_init (dtplayer_para_t * para);
	int dtplayer_start ();
	int dtplayer_pause ();
	int dtplayer_resume ();
	int dtplayer_stop ();
	int dtplayer_seek (int s_time);
	int dtplayer_get_states (player_state_t * state);
};

dtplayer *open_player_module();

#endif
