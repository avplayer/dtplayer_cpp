#include "dthost.h"
#include "dtport_api.h"
#include "dtaudio_api.h"
#include "dtvideo_api.h"
#include "dt_macro.h"

#include "unistd.h"

#define TAG "host-MGT"

dthost_context::dthost_context(dthost_para_t &_para)
{
	para.has_audio = _para.has_audio;
	para.has_sub = _para.has_sub;
	para.has_video = _para.has_video;
	
	para.audio_bitrate = _para.audio_bitrate;
	para.audio_block_align = _para.audio_block_align;
	para.audio_channel = _para.audio_channel;
	para.audio_codec_id = _para.audio_codec_id;
	para.audio_den = _para.audio_den;
	para.audio_num = _para.audio_num;
	para.audio_dst_channels = _para.audio_dst_channels;
	para.audio_dst_samplerate = _para.audio_dst_samplerate;
	para.audio_extra_size = _para.audio_extra_size;
	for(int i = 0; i < para.audio_extra_size; i++)
		para.audio_extra_data[i] = _para.audio_extra_data[i];
	para.audio_filter = _para.audio_filter;
	para.audio_format = _para.audio_format;
	para.audio_output = _para.audio_output;
	para.audio_sample_fmt = _para.audio_sample_fmt;
	para.audio_samplerate = _para.audio_samplerate;
	para.actx_priv = _para.actx_priv;
	
	para.ratio64 = _para.ratio64;
	para.sub_format = _para.sub_format;
	para.sub_height = _para.sub_height;
	para.sub_id = _para.sub_id;
	para.sub_width = _para.sub_width;
	para.sync_enable = _para.sync_enable;
	

	para.video_den = _para.video_den;
	para.video_num = para.video_num;
	para.video_src_height = _para.video_src_height;
	para.video_src_pixfmt = _para.video_src_pixfmt;
	para.video_src_width = _para.video_src_width;
	para.video_dest_height = _para.video_dest_height;
	para.video_dest_pixfmt = _para.video_dest_pixfmt;
	para.video_dest_width = _para.video_dest_width;
	
	para.video_extra_size = _para.video_extra_size;
	for(int i = 0; i < para.video_extra_size; i++)
		para.video_extra_data[i] = _para.video_extra_data[i];
	para.video_filter = _para.video_filter;
	para.video_format = _para.video_format;
	para.video_fps = _para.video_fps;

	para.video_output = _para.video_output;
	para.video_rate = _para.video_rate;
	para.video_ratio = _para.video_ratio;
	
	para.vctx_priv = _para.vctx_priv;
	
	mod_audio = nullptr;
}


int dthost_context::host_sync_enable ()
{
    return this->sync_enable && (this->sync_mode == DT_SYNC_AUDIO_MASTER);
}

int64_t dthost_context::host_get_apts ()
{
    return this->pts_audio;
}

int64_t dthost_context::host_get_vpts ()
{
    return this->pts_video;
}

int64_t dthost_context::host_get_systime ()
{
    return this->sys_time;
}

int64_t dthost_context::host_get_avdiff ()
{
    int64_t atime = -1;
    int64_t vtime = -1;
    atime = this->pts_audio;
    vtime = this->pts_video;
    this->av_diff = (atime - vtime) / 90; //ms
    return this->av_diff;
}

int dthost_context::host_update_apts (int64_t apts)
{
    this->pts_audio = apts;
    dt_debug (TAG, "update apts:%llx \n", apts);
    if (!this->para.has_video)
    {
        this->sys_time = apts;
        return 0;
    }
    if (!this->para.sync_enable) //sync disable, video will correct systime
        return 0;
    //maybe need to correct sys clock
    int64_t vpts = this->host_get_vpts ();
    int64_t sys_time = this->host_get_systime ();
    int64_t avdiff = this->host_get_avdiff ();
    int64_t asdiff = (llabs) (apts - this->sys_time) / 90; //apts sys_time diff

    if (sys_time == -1)         //if systime have not been set,wait
        return 0;

    if (this->host_sync_enable () && avdiff / 90 > AVSYNC_THRESHOLD_MAX) //close sync
    {
        dt_info (TAG, "avdiff:%lld ecceed :%d ms, cloase sync \n", avdiff / 90, AVSYNC_THRESHOLD_MAX);
        this->sync_mode = DT_SYNC_VIDEO_MASTER;
        return 0;
    }

    if (this->sync_enable && this->sync_mode == DT_SYNC_VIDEO_MASTER && avdiff / 90 < AVSYNC_THRESHOLD_MAX) // enable sync again
        this->sync_mode = DT_SYNC_AUDIO_MASTER;

    if (avdiff < AVSYNC_THRESHOLD)
        return 0;
    if (this->host_sync_enable ())
    {
        dt_info (TAG, "[%s:%d] correct sys time apts:%lld vpts:%lld sys_time:%lld AVDIFF:%d ASDIFF:%d\n", __FUNCTION__, __LINE__, apts, vpts, sys_time, avdiff, asdiff);
        this->sys_time = apts;
    }
    return 0;
}

int dthost_context::host_update_vpts (int64_t vpts)
{
    this->pts_video = vpts;
    dt_debug (TAG, "update vpts:%llx \n", vpts);
    //maybe need to correct sys clock
    int64_t sys_time = this->host_get_systime ();
    int64_t avdiff = this->host_get_avdiff ();
    //int64_t vsdiff = llabs(vpts - this->sys_time)/90; //vpts sys_time diff

    if (sys_time == -1)
        return 0;
    if (!this->sync_enable && avdiff / 90 > AVSYNC_THRESHOLD)
    {
        dt_info (TAG, "[%s:%d] sync disable or avdiff too much, update systime with vpts, sys:%lld vpts:%lld\n", __FUNCTION__, __LINE__, sys_time, vpts);
        this->sys_time = vpts;
        return 0;
    }

    if (this->host_sync_enable () && avdiff / 90 > AVSYNC_THRESHOLD_MAX) //close sync
    {
        dt_info (TAG, "avdiff:%d ecceed :%d ms, cloase sync \n", avdiff / 90, AVSYNC_THRESHOLD_MAX);
        this->sync_mode = DT_SYNC_VIDEO_MASTER;
        return 0;
    }

    if (this->sync_enable && this->sync_mode == DT_SYNC_VIDEO_MASTER && avdiff / 90 < AVSYNC_THRESHOLD_MAX)
        this->sync_mode = DT_SYNC_AUDIO_MASTER;

    return 0;
}

int dthost_context::host_update_systime (int64_t sys_time)
{
    this->sys_time = sys_time;
    return 0;
}

int64_t dthost_context::host_get_current_time ()
{
    int64_t ctime = -1;
    int64_t atime = -1;
    int64_t vtime = -1;

    if (this->para.has_audio)
        atime = this->pts_audio;
    if (this->para.has_video)
        vtime = this->pts_video;
    ctime = this->sys_time;
    dt_debug (TAG, "ctime:%llx atime:%llx vtime:%llx sys_time:%llx av_diff:%x sync mode:%x\n", ctime, atime, vtime, this->sys_time, this->av_diff, this->av_sync);
    return ctime;
}

//==Part2:Control
int dthost_context::host_start ()
{
    int ret;
    int has_audio, has_video, has_sub;
    has_audio = this->para.has_audio;
    has_video = this->para.has_video;
    has_sub = this->para.has_sub;

    /*check start condition */
    int audio_start_flag = 0;
    int video_start_flag = 0;
    int64_t first_apts = -1;
    int64_t first_vpts = -1;
    dt_info (TAG, "check start condition has_audio:%d has_video:%d has_sub:%d\n", has_audio, has_video, has_sub);
    int print_cnt = 100;
    do
    {
        if (!has_audio)
            audio_start_flag = 1;
        else
            audio_start_flag = !((first_apts = dtaudio_get_first_pts (this->audio_priv)) == -1);
        if (!has_video)
            video_start_flag = 1;
        else
            video_start_flag = !((first_vpts = dtvideo_get_first_pts (this->video_priv)) == -1);
        if (audio_start_flag && video_start_flag)
            break;
        usleep (1000);
        if(print_cnt-- == 0)
        {
            dt_info (TAG, "audio:%d video:%d \n", audio_start_flag, video_start_flag);
            print_cnt = 100;
        }
    }
    while (1);

    dt_info (TAG, "first apts:%lld first vpts:%lld \n", first_apts, first_vpts);
    this->pts_audio = first_apts;
    this->pts_video = first_vpts;

    int drop_flag = 0;
    int av_diff_ms = abs (this->pts_video - this->pts_audio) / 90;
    if (av_diff_ms > AV_DROP_THRESHOLD)
    {
        dt_info (TAG, "FIRST AV DIFF EXCEED :%d ms,DO NOT DROP\n",AV_DROP_THRESHOLD);
        this->sync_mode = DT_SYNC_VIDEO_MASTER;
    }
    drop_flag = (av_diff_ms > 100 && av_diff_ms < AV_DROP_THRESHOLD); // exceed 100ms
    if (!this->host_sync_enable ())
        drop_flag = 0;
    if (drop_flag)
    {
        if (this->pts_audio > this->pts_video)
            dtvideo_drop (this->video_priv, this->pts_audio);
        else
            dtaudio_drop (this->audio_priv, this->pts_video);
    }
    dt_info (TAG, "apts:%lld vpts:%lld \n", this->pts_audio, this->pts_video);

    if (has_audio)
    {
        ret = dtaudio_start (this->audio_priv);
        if (ret < 0)
        {
            dt_error (TAG, "[%s:%d] dtaudio start failed \n", __FUNCTION__, __LINE__);
            return -1;
        }
        dt_info (TAG, "[%s:%d] dtaudio start ok \n", __FUNCTION__, __LINE__);
    }
    if (has_video)
    {
        ret = dtvideo_start (this->video_priv);
        if (ret < 0)
        {
            dt_error (TAG, "[%s:%d] dtvideo start failed \n", __FUNCTION__, __LINE__);
            return -1;
        }
        dt_info (TAG, "[%s:%d]video start ok\n", __FUNCTION__, __LINE__);
    }

    return 0;
}

int dthost_context::host_pause ()
{
    int ret;
    int has_audio, has_video, has_sub;
    has_audio = this->para.has_audio;
    has_video = this->para.has_video;
    has_sub = this->para.has_sub;

    if (has_audio)
    {
        ret = dtaudio_pause (this->audio_priv);
        if (ret < 0)
            dt_error (TAG, "[%s:%d] dtaudio external pause failed \n", __FUNCTION__, __LINE__);
    }
    if (has_video)
        ret = dtvideo_pause (this->video_priv);
    if (has_sub)
        ;                       //ret = dtsub_pause();
    return 0;
}

int dthost_context::host_resume ()
{
    int ret;
    int has_audio, has_video, has_sub;
    has_audio = this->para.has_audio;
    has_video = this->para.has_video;
    has_sub = this->para.has_sub;

    if (has_audio)
    {
        ret = dtaudio_resume (this->audio_priv);
        if (ret < 0)
            dt_error (TAG, "[%s:%d] dtaudio external pause failed \n", __FUNCTION__, __LINE__);
    }
    if (has_video)
        ret = dtvideo_resume (this->video_priv);
    if (has_sub)
        ;

    return 0;
}

int dthost_context::host_stop ()
{
    int ret;
    int has_audio, has_video, has_sub;
    has_audio = this->para.has_audio;
    has_video = this->para.has_video;
    has_sub = this->para.has_sub;

    /*first stop audio module */
    if (has_audio)
    {
        ret = dtaudio_stop (this->audio_priv);
        if (ret < 0)
            dt_error (TAG, "[%s:%d] dtaudio stop failed \n", __FUNCTION__, __LINE__);
    }
    /*stop video mudule */
    if (has_video)
    {
        ret = dtvideo_stop (this->video_priv);
        if (ret < 0)
            dt_error (TAG "[%s:%d] dtvideo stop failed \n", __FUNCTION__, __LINE__);
    }
    if (has_sub)
        ;
    /*stop dtport module at last */
    ret = dtport_stop (this->port_priv);
    if (ret < 0)
        dt_error (TAG, "[%s:%d] dtport stop failed \n", __FUNCTION__, __LINE__);
    return 0;
}

int dthost_context::host_init ()
{
    int ret;
    dthost_para_t *host_para = &this->para;
    /*
     *set sync mode
     * read from property file, get sync mode setted by user
     * if user not set ,get by default
     */
	this->sync_enable = host_para->sync_enable;
    if (host_para->has_audio && host_para->has_video)
        this->av_sync = DT_SYNC_AUDIO_MASTER;
    else
	{
		this->sync_enable = 0;
        this->av_sync = DT_SYNC_VIDEO_MASTER;
	}
    this->av_diff = 0;
    this->pts_audio = this->pts_video = this->sys_time = -1;

    /*init port */
    dtport_para_t port_para;
    port_para.has_audio = host_para->has_audio;
    port_para.has_subtitle = host_para->has_sub;
    port_para.has_video = host_para->has_video;

    ret = dtport_init (&this->port_priv, &port_para, this);
    if (ret < 0)
        goto ERR1;
    dt_info (TAG, "[%s:%d] dtport init success \n", __FUNCTION__, __LINE__);

    /*init video */
    if (host_para->has_video)
    {
        dtvideo_para_t video_para;
        video_para.vfmt = host_para->video_format;
        video_para.d_width = host_para->video_dest_width;
        video_para.d_height = host_para->video_dest_height;
        video_para.s_width = host_para->video_src_width;
        video_para.s_height = host_para->video_src_height;
        video_para.s_pixfmt = host_para->video_src_pixfmt;
        video_para.d_pixfmt = DTAV_PIX_FMT_YUV420P;
        video_para.rate = host_para->video_rate;
        video_para.ratio = host_para->video_ratio;
        video_para.fps = host_para->video_fps;
        video_para.num = host_para->video_num;
        video_para.den = host_para->video_den;

        video_para.extradata_size = host_para->video_extra_size;
        if (video_para.extradata_size)
        {
			for(int i = 0; i < video_para.extradata_size; i++)
				video_para.extradata[i] = host_para->video_extra_data[i];
        }
        video_para.video_filter = host_para->video_filter;
        video_para.video_output = host_para->video_output;
        video_para.avctx_priv = host_para->vctx_priv;
        ret = dtvideo_init (&this->video_priv, &video_para, this);
        if (ret < 0)
            goto ERR3;
        dt_info (TAG, "[%s:%d] dtvideo init success \n", __FUNCTION__, __LINE__);
        if (!this->video_priv)
        {
            dt_error (TAG, "[%s:%d] dtvideo init failed video_priv ==NULL \n", __FUNCTION__, __LINE__);
            goto ERR3;
        }
    }

    /*init audio */
    if (host_para->has_audio)
    {
        dtaudio_para_t audio_para;
        audio_para.afmt = host_para->audio_format;
        audio_para.bps = host_para->audio_sample_fmt;
        audio_para.channels = host_para->audio_channel;
        audio_para.dst_channels = host_para->audio_dst_channels;
        audio_para.dst_samplerate = host_para->audio_dst_samplerate;
        audio_para.data_width = host_para->audio_sample_fmt;
        audio_para.samplerate = host_para->audio_samplerate;
        audio_para.num = host_para->audio_num;
        audio_para.den = host_para->audio_den;

        audio_para.extradata_size = host_para->audio_extra_size;
        if (host_para->audio_extra_size)
        {
            memset (audio_para.extradata, 0, AUDIO_EXTRADATA_SIZE);
			for(int i = 0; i < audio_para.extradata_size; i++)
				audio_para.extradata[i] = host_para->audio_extra_data[i];
        }
        audio_para.audio_filter = host_para->audio_filter;
        audio_para.audio_output = host_para->audio_output;
        audio_para.avctx_priv = host_para->actx_priv;
		
		mod_audio = open_audio_module();
		ret = mod_audio->init(&audio_para,this);		
        //ret = dtaudio_init (&this->audio_priv, &audio_para, this);
        if (ret < 0)
            goto ERR2;
        dt_info (TAG, "[%s:%d]dtaudio init success \n", __FUNCTION__, __LINE__);
        if (!this->audio_priv)
        {
            dt_error (TAG, "[%s:%d] dtaudio init failed audio_priv ==NULL \n", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    /*init sub */
    return 0;
  ERR1:
    return -1;
  ERR2:
    dtport_stop (this->port_priv);
    return -2;
  ERR3:
    dtport_stop (this->port_priv);
    if (host_para->has_audio)
    {
        dtaudio_stop (this->audio_priv);
    }
    return -3;
}

int dthost_context::host_write_frame (dt_av_frame_t * frame, int type)
{
    return dtport_write_frame (this->port_priv, frame, type);
}

int dthost_context::host_read_frame (dt_av_frame_t * frame, int type)
{
    if (this == NULL)
    {
        dt_error (TAG, "dthost_context is NULL\n");
        return -1;
    }
    if (NULL == this->port_priv)
    {
        dt_error (TAG, "dtport is NULL\n");
        return -1;
    }
    return dtport_read_frame (this->port_priv, frame, type);
}

int dthost_context::host_get_state (host_state_t * state)
{
    int has_audio = this->para.has_audio;
    int has_video = this->para.has_video;
    buf_state_t buf_state;
    dec_state_t dec_state;
    if (has_audio)
    {
        dtport_get_state (this->port_priv, &buf_state, DT_TYPE_AUDIO);
        dtaudio_get_state (this->audio_priv, &dec_state);
        state->abuf_level = buf_state.data_len;
		state->apkt_size = buf_state.size;
        state->adec_err_cnt = dec_state.adec_error_count;
        state->cur_apts = this->pts_audio;
    }
    else
    {
        state->abuf_level = -1;
		state->apkt_size = -1;
        state->cur_apts = -1;
        state->adec_err_cnt = -1;
    }

    if (has_video)
    {
        dtport_get_state (this->port_priv, &buf_state, DT_TYPE_VIDEO);
        dtvideo_get_state (this->video_priv, &dec_state);
        state->vbuf_level = buf_state.data_len;
		state->vpkt_size = buf_state.size;
        state->vdec_err_cnt = dec_state.vdec_error_count;
        state->cur_vpts = this->pts_video;
    }
    else
    {
        state->vbuf_level = -1;
		state->vpkt_size = -1;
        state->cur_vpts = -1;
        state->vdec_err_cnt = -1;
    }
    dt_debug (TAG, "[%s:%d] apts:%lld vpts:%lld systime:%lld tsync_enable:%d sync_mode:%d \n", __FUNCTION__, __LINE__, this->pts_audio, this->pts_video, this->sys_time, this->sync_enable, this->sync_mode);
    state->cur_systime = this->sys_time;
    return 0;
}

int dthost_context::host_get_out_closed ()
{
    int aout_close = 0;
    int vout_close = 0;
    if (this->para.has_audio)
        aout_close = dtaudio_get_out_closed (this->audio_priv);
    else
        aout_close = 1;
    if (this->para.has_video)
        vout_close = dtvideo_get_out_closed (this->video_priv);
    else
        vout_close = 1;
    if (aout_close && vout_close)
        dt_info (TAG, "AV OUT CLOSED \n");
    return (aout_close && vout_close);
}
