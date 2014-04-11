#include "dtvideo.h"
#include "dthost_api.h"

#define TAG "VIDEO-MGT"

void video_register_all()
{
    vdec_register_all();
    vout_register_all();
}

void register_ext_vo(vo_wrapper_t &vo)
{
   vout_register_ext(vo); 
}

/*read frame from dtport*/
int dtvideo_context::dtvideo_read_frame (dt_av_frame_t * frame)
{
    int type = DT_TYPE_VIDEO;
    int ret = 0;
	module_video *video = this->parent;
	dthost *host = video->host_ext;
    ret = host->read_frame (frame, type);
    return ret;
}

/*get picture from vo_queue,remove*/
AVPicture_t * dtvideo_context::dtvideo_output_read ()
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) this;
	AVPicture_t *pic = nullptr;
	vctx->mux_vo_queue.lock();
	if(!vctx->queue_vo.empty())
	{
		pic = vctx->queue_vo.front();
		vctx->queue_vo.pop();
	}
	vctx->mux_vo_queue.unlock();
   
    return pic;
}

/*pre get picture from vo_queue, not remove*/
AVPicture_t * dtvideo_context::dtvideo_output_pre_read ()
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) this;
	AVPicture_t *pic = nullptr;
	vctx->mux_vo_queue.lock();
	if(!vctx->queue_vo.empty())
	{
		pic = vctx->queue_vo.front();
	}
	vctx->mux_vo_queue.unlock();
   
    return pic;
}

int64_t dtvideo_context::dtvideo_get_systime ()
{
    dtvideo_context_t *vctx = this;
    if (vctx->video_status <= VIDEO_STATUS_INITED)
        return -1;
	module_video *video = this->parent;
	dthost *host = video->host_ext;
    return host->get_systime ();
}

void dtvideo_context::dtvideo_update_systime (int64_t sys_time)
{
    dtvideo_context_t *vctx = this;
    if (vctx->video_status <= VIDEO_STATUS_INITED)
        return;
	module_video *video = this->parent;
	dthost *host = video->host_ext;
    host->update_systime (sys_time);
    return;
}

void dtvideo_context::dtvideo_update_pts ()
{
    dtvideo_context_t *vctx = this;
    if (vctx->video_status < VIDEO_STATUS_INITED)
        return;
		module_video *video = this->parent;
	dthost *host = video->host_ext;
    host->update_vpts (vctx->current_pts);
    return;
}

dtvideo_context::dtvideo_context(dtvideo_para_t& _para)
{
	video_para.d_height = _para.d_height;
	video_para.d_width = _para.d_width;
	video_para.d_pixfmt = _para.d_pixfmt;
	video_para.s_height = _para.s_height;
	video_para.s_pixfmt = _para.s_pixfmt;
	video_para.s_width = _para.s_width;
	video_para.vfmt = _para.vfmt;
	
	video_para.den = _para.den;
	video_para.num = _para.num;
	
	video_para.extradata_size = _para.extradata_size;
	for(int i = 0; i< video_para.extradata_size; i ++)
		video_para.extradata[i] = _para.extradata[i];
	
	video_para.fps = _para.fps;
	video_para.rate = _para.rate;
	video_para.ratio = _para.ratio;
	
	
	video_para.video_filter = _para.video_filter;
	video_para.video_output = _para.video_output;
	
	video_para.avctx_priv = _para.avctx_priv;
	
	this->video_status = VIDEO_STATUS_IDLE;
}

#if 0
int dtvideo_get_avdiff (void *priv)
{
    dtvideo_context_t *vctx = (dtvideo_context_t *) priv;
    return dthost_get_avdiff (vctx->parent);
}
#endif

int64_t dtvideo_context::video_get_current_pts ()
{
    uint64_t pts;
    if (this->video_status <= VIDEO_STATUS_INITED)
        return -1;
    pts = this->current_pts;
    return pts;
}

int64_t dtvideo_context::video_get_first_pts ()
{
    if (this->video_status != VIDEO_STATUS_INITED)
        return -1;
    dt_debug (TAG, "fitst vpts:%lld \n", this->video_dec->pts_first);
    return this->video_dec->pts_first;
}

int dtvideo_context::video_drop (int64_t target_pts)
{
    int64_t diff = target_pts - this->video_get_first_pts ();
    int diff_time = diff / 90;
    if (diff_time > AV_DROP_THRESHOLD)
    {
        dt_info (TAG, "diff time exceed %d ms, do not drop!\n", diff_time);
        return 0;
    }
    dt_info (TAG, "[%s:%d]target pts:%lld \n", __FUNCTION__, __LINE__, target_pts);
    AVPicture_t *pic = NULL;
	int64_t cur_pts = this->video_get_first_pts();
    int drop_count = 300;
    do
    {
        pic = this->dtvideo_output_read ();
        if (!pic)
        {
            if (drop_count-- == 0)
            {
                dt_info (TAG, "3s read nothing, quit drop video\n");
                break;
            }
            usleep (10000);
            continue;
        }
        drop_count = 300;
        cur_pts = pic->pts;
        dt_debug (TAG, "read pts:%lld \n", pic->pts);
        free (pic);
        pic = NULL;
        if (cur_pts > target_pts)
            break;
    }
    while (1);
    dt_info (TAG, "video drop finish,drop count:%d \n", drop_count);
    this->current_pts = cur_pts;
	this->dtvideo_update_pts();
    return 0;
}

int dtvideo_context::video_get_dec_state (dec_state_t * dec_state)
{
    if (this->video_status <= VIDEO_STATUS_INITED)
        return -1;
    return -1;
    dtvideo_decoder_t *vdec = this->video_dec;
    dec_state->vdec_error_count = vdec->decode_err_cnt;
    dec_state->vdec_fps = vdec->para.rate;
    dec_state->vdec_width = vdec->para.d_width;
    dec_state->vdec_height = vdec->para.d_height;
    dec_state->vdec_status = vdec->status;
    return 0;
}

//reserve 10 frame
//if low level just return
int dtvideo_context::video_get_out_closed ()
{
    return 1;
}

int dtvideo_context::video_start ()
{
    if (this->video_status == VIDEO_STATUS_INITED)
    {
        dtvideo_output_t *video_out = this->video_out;
		video_out->video_output_start();
        this->video_status = VIDEO_STATUS_ACTIVE;
    }
    else
        dt_error (TAG, "[%s:%d]video output start failed \n", __FUNCTION__, __LINE__);
    return 0;
}

int dtvideo_context::video_pause ()
{
    if (this->video_status == VIDEO_STATUS_ACTIVE)
    {
        dtvideo_output_t *video_out = this->video_out;
		video_out->video_output_pause();
        this->video_status = VIDEO_STATUS_PAUSED;
    }
    return 0;
}

int dtvideo_context::video_resume ()
{
    if (this->video_status == VIDEO_STATUS_PAUSED)
    {
        dtvideo_output_t *video_out = this->video_out;
		video_out->video_output_resume();
        this->video_status = VIDEO_STATUS_ACTIVE;
        return 0;
    }
    return -1;
}

int dtvideo_context::video_stop ()
{
    if (this->video_status > VIDEO_STATUS_INITED)
    {
        dtvideo_output_t *video_out = this->video_out;
		video_out->video_output_stop();
		delete(video_out);
		this->video_out = nullptr;
		
        dtvideo_decoder_t *video_decoder = this->video_dec;
		this->video_dec->video_decoder_stop();
		delete(video_decoder);
		this->video_dec = nullptr;
        this->video_status = VIDEO_STATUS_STOPPED;
    }
    return 0;
}

int dtvideo_context::video_init ()
{
    int ret = 0;
	dtvideo_output_t *video_out = nullptr;
    dt_info (TAG, "[%s:%d] dtvideo_mgt start init\n", __FUNCTION__, __LINE__);
    //call init
    this->video_status = VIDEO_STATUS_INITING;
    this->current_pts = this->last_valid_pts = -1;
	
	dtvideo_para_t &para = this->video_para;
	this->video_dec = new dtvideo_decoder(para);
	dtvideo_decoder_t *video_dec = this->video_dec;
    video_dec->parent = this;
	ret = video_dec->video_decoder_init();
    if (ret < 0)
        goto err1;
	
	this->video_out = new dtvideo_output(para);
	video_out = this->video_out;
    video_out->parent = this;
	ret = video_out->video_output_init(para.video_output);
    if (ret < 0)
        goto err2;

    this->video_status = VIDEO_STATUS_INITED;
    dt_info (TAG, "dtvideo init ok,status:%d \n", this->video_status);
    return 0;
  err1:
    dt_info (TAG, "[%s:%d]video decoder init failed \n", __FUNCTION__, __LINE__);
    return -1;
  err2:
	video_dec->video_decoder_stop();
	delete(video_dec);
    dt_info (TAG, "[%s:%d]video output init failed \n", __FUNCTION__, __LINE__);
    return -3;
}
