#include "dtplayer.h"
#include "dtdemuxer_api.h"

#define TAG "PLAYER-IO"
//status
#define IO_LOOP_PAUSED 0
#define IO_LOOP_RUNNING 1
#define IO_LOOP_QUIT 2
//flag
#define IO_FLAG_NULL 0
#define IO_FLAG_PAUSE 1

static void *player_io_thread (dtplayer_context_t * dtp_ctx);

static int player_read_frame (dtplayer_context_t * dtp_ctx, dt_av_frame_t * frame)
{
	return dtp_ctx->demux_ext->read_frame(frame);
}

static int player_write_frame (dtplayer_context_t * dtp_ctx, dt_av_frame_t * frame)
{
	dthost *host = dtp_ctx->host_ext;
    return host->write_frame (frame, frame->type);
}


int dtplayer_context::start_io_thread ()
{
    this->io_loop.status = IO_LOOP_PAUSED;
    this->io_loop.flag = IO_FLAG_NULL;
    this->io_loop.io_loop_thread = std::thread(player_io_thread,this);
    this->io_loop.status = IO_LOOP_RUNNING;
    dt_info (TAG, "IO Thread start ok\n");
    return 0;
}

int dtplayer_context::pause_io_thread ()
{
    this->io_loop.flag = IO_FLAG_PAUSE;
    while (this->io_loop.status != IO_LOOP_PAUSED)
        usleep (100);
    this->io_loop.flag = IO_FLAG_NULL;
    dt_info(TAG,"IO THREAD PAUSE\n");
    return 0;
}

int dtplayer_context::resume_io_thread ()
{
    this->io_loop.flag = IO_FLAG_NULL;
    this->io_loop.status = IO_LOOP_RUNNING;
    dt_info(TAG,"IO THREAD RESUME\n");
    return 0;
}

int dtplayer_context::stop_io_thread ()
{
    this->io_loop.flag = IO_FLAG_NULL;
    this->io_loop.status = IO_LOOP_QUIT;
    this->io_loop.io_loop_thread.join();
    dt_info(TAG,"IO THREAD QUIT OK\n");
    return 0;
}

static void *player_io_thread (dtplayer_context_t * dtp_ctx)
{
    io_loop_t *io_ctl = &dtp_ctx->io_loop;
    dt_av_frame_t *frame = nullptr;
    int frame_valid = 0;
    int ret = 0;
    do
    {
        usleep (100);
        if (io_ctl->flag == IO_FLAG_PAUSE)
            io_ctl->status = IO_LOOP_PAUSED;
        if (io_ctl->status == IO_LOOP_QUIT)
            goto QUIT;
        if (io_ctl->status == IO_LOOP_PAUSED)
        {
            //when pause read thread,we need skip currnet pkt
            if (frame_valid == 1)
			{
				free (frame->data);
				delete(frame);
			}
            frame_valid = 0;
            usleep (100);
            continue;
        }
        /*io ops */
        if (frame_valid == 1)
            goto WRITE_FRAME;
        
		frame = new dt_av_frame_t;
		
        ret = player_read_frame (dtp_ctx, frame);
        if (ret == DTERROR_NONE)
            frame_valid = 1;
        else
        {
            if (ret == DTERROR_READ_EOF)
            {
                io_ctl->status = IO_LOOP_QUIT;
                dtp_ctx->ctrl_info.eof_flag = 1;
            }
            delete(frame);
            usleep (100);
            continue;
        }
        dt_debug (TAG, "read ok size:%d pts:%lld \n",frame->size,frame->pts);
      WRITE_FRAME:
        ret = player_write_frame (dtp_ctx, frame);
        if (ret == DTERROR_NONE)
        {
            dt_debug (TAG, "player write ok size:%d \n",frame->size);
			frame = nullptr;
            frame_valid = 0;
        }
        else
		{
            dt_debug (TAG, "write frame failed , write again \n");
		}
    }
    while (1);
  QUIT:
    //dt_info (TAG, "io thread quit ok\n");
    return 0;
}
