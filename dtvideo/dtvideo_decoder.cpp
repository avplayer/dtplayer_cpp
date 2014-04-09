#include <vector>
#include "dtvideo.h"
#include "dtvideo_decoder.h"

//#define DTVIDEO_DECODER_DUMP 0

#define TAG "VIDEO-DEC"

#define REGISTER_VDEC(X,x)	 	\
	{							\
		extern vd_wrapper_t vdec_##x##_ops; 	\
		register_vdec(vdec_##x##_ops); 	\
	}

static std::vector<vd_wrapper_t> g_vd;

static void register_vdec (vd_wrapper_t &vdec)
{
	g_vd.push_back(vdec);
    dt_info (TAG, "[%s:%d] register vdec, name:%s fmt:%d \n", __FUNCTION__, __LINE__, vdec.name, vdec.vfmt);
}

void vdec_register_all ()
{
    //comments:video using ffmpeg decoder only
#ifdef ENABLE_VDEC_FFMPEG
    REGISTER_VDEC (FFMPEG, ffmpeg);
#endif
    return;
}

static int select_video_decoder (dtvideo_decoder_t * decoder)
{
    if (g_vd.empty())
    {
        dt_error (TAG, "[%s:%d] select no video decoder \n", __FUNCTION__, __LINE__);
        return -1;
    }
    decoder->wrapper = & g_vd[0];
    dt_info (TAG, "[%s:%d] select--%s video decoder \n", __FUNCTION__, __LINE__, decoder->wrapper->name);
    return 0;
}

static int64_t pts_exchange (dtvideo_decoder_t * decoder, int64_t pts)
{
    return pts;
}

dtvideo_decoder::dtvideo_decoder(dtvideo_para_t& _para)
{
	para.d_height = _para.d_height;
	para.d_width = _para.d_width;
	para.d_pixfmt = _para.d_pixfmt;
	para.s_height = _para.s_height;
	para.s_pixfmt = _para.s_pixfmt;
	para.s_width = _para.s_width;
	para.vfmt = _para.vfmt;
	
	para.den = _para.den;
	para.num = _para.num;
	
	para.extradata_size = _para.extradata_size;
	for(int i = 0; i< para.extradata_size; i ++)
		para.extradata[i] = _para.extradata[i];
	
	para.fps = _para.fps;
	para.rate = _para.rate;
	para.ratio = _para.ratio;
	
	
	para.video_filter = _para.video_filter;
	para.video_output = _para.video_output;
	
	para.avctx_priv = _para.avctx_priv;
	
	this->status = VDEC_STATUS_IDLE;
}

static void *video_decode_loop (void *arg)
{
    dt_av_frame_t frame;
    dtvideo_decoder_t *decoder = (dtvideo_decoder_t *) arg;
    vd_wrapper_t *wrapper = decoder->wrapper;
    dtvideo_context_t *vctx = (dtvideo_context_t *) decoder->parent;

    /*used for decode */
    AVPicture_t *picture = NULL;
    int ret;
    dt_info (TAG, "[%s:%d] start decode loop \n", __FUNCTION__, __LINE__);

    do
    {
        if (decoder->status == VDEC_STATUS_IDLE)
        {
            dt_info (TAG, "[%s:%d] Idle status ,please wait \n", __FUNCTION__, __LINE__);
            usleep (1000);
            continue;
        }
        if (decoder->status == VDEC_STATUS_EXIT)
        {
            dt_info (TAG, "[%s:%d] receive decode loop exit cmd \n", __FUNCTION__, __LINE__);
            break;
        }

        /*read frame */
        if (!decoder->parent)
        {
            usleep (1000);
            continue;
        }

        if (vctx->queue_vo.size() >= VIDEO_OUT_MAX_COUNT)
        {
            //vo queue full
            usleep (1000);
            continue;
        }
        ret = vctx->dtvideo_read_frame(&frame);
        if (ret < 0)
        {
            usleep (1000);
            dt_debug (TAG, "[%s:%d] dtaudio decoder loop read frame failed \n", __FUNCTION__, __LINE__);
            continue;
        }

        /*read one frame,enter decode frame module */
        //will exec once for one time
        ret = wrapper->decode_frame (wrapper, &frame, &picture);
        if (ret <= 0)
        {
            decoder->decode_err_cnt++;
            dt_error (TAG, "[%s:%d]decode failed \n", __FUNCTION__, __LINE__);
            picture = NULL;
            goto DECODE_END;
        }
        if (!picture)
            goto DECODE_END;
		
        if (picture->pts >= 0 && decoder->pts_first == -1)
        {
            //use first decoded pts to estimate pts
            dt_info (TAG, "[%s:%d]first frame: pts:%llu dts:%llu duration:%d size:%d\n", __FUNCTION__, __LINE__, frame.pts, frame.dts, frame.duration, frame.size);
            decoder->pts_first = pts_exchange (decoder, picture->pts);

        }
        decoder->frame_count++;
        //Got one frame
        //picture->pts = frame.pts;
        /*queue in */
		vctx->mux_vo_queue.lock();
		vctx->queue_vo.push(picture);
		vctx->mux_vo_queue.unlock();
        picture = NULL;
      DECODE_END:
        //we successfully decodec one frame
        if (frame.data)
        {
			free (frame.data);
			frame.data = nullptr;
			frame.size = 0;
        }
    }
    while (1);
    dt_info (TAG, "[%s:%d]decoder loop thread exit ok\n",__FUNCTION__, __LINE__);
    return NULL;
}

int dtvideo_decoder::video_decoder_init ()
{
    int ret = 0;    
    /*select decoder */
    ret = select_video_decoder (this);
    if (ret < 0)
        return -1;
	vd_wrapper_t *wrapper = this->wrapper;
    /*init decoder */
    this->pts_current = this->pts_first = -1;
    this->decoder_priv = this->para.avctx_priv;
    ret = wrapper->init (wrapper,this);
    if (ret < 0)
        return -1;

    dt_info (TAG, "[%s:%d] video decoder init ok\n", __FUNCTION__, __LINE__);
    video_decoder_thread = std::thread(video_decode_loop,this);
	video_decoder_start();
    return 0;
}

int dtvideo_decoder::video_decoder_start ()
{
    this->status = VDEC_STATUS_RUNNING;
    return 0;
}

void dtpicture_free (void *pic)
{

    AVPicture_t *picture = (AVPicture_t *) (pic);
    if (picture->data)
        free (picture->data[0]);
    return;
}

int dtvideo_decoder::video_decoder_stop ()
{
    /*Decode thread exit */
	vd_wrapper_t *wrapper = this->wrapper;
    this->status = VDEC_STATUS_EXIT;
	this->video_decoder_thread.join();
    wrapper->release (wrapper);
    /*uninit buf */
	dtvideo_context_t *vctx = (dtvideo_context_t *)this->parent;
    vctx->mux_vo_queue.lock();
	while(!vctx->queue_vo.empty())
	{
		AVPicture_t *pic = vctx->queue_vo.front();
		dtpicture_free(pic);
		delete(pic);
		vctx->queue_vo.pop();
	}
	vctx->mux_vo_queue.unlock();
    
    return 0;
}
