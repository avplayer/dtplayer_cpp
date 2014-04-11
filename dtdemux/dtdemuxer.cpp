
#include <vector>

#include "dtdemuxer.h"
#include "dtstream_api.h"

#define TAG "DEMUXER"

#define REGISTER_DEMUXER(X,x) \
    {                         \
        extern demuxer_wrapper_t demuxer_##x; \
        register_demuxer(demuxer_##x);     \
    }
static std::vector<demuxer_wrapper_t> demuxer_wrappers;

static void register_demuxer (const demuxer_wrapper_t & wrapper)
{
	demuxer_wrappers.push_back(wrapper);
}

void demuxer_register_all ()
{
    REGISTER_DEMUXER (AAC, aac);
#ifdef ENABLE_DEMUXER_FFMPEG
    REGISTER_DEMUXER (FFMPEG, ffmpeg);
#endif
}

static int demuxer_select (dtdemuxer_context_t * dem_ctx)
{
    if (demuxer_wrappers.empty())
        return -1;
    int score = 0;

    std::vector< demuxer_wrapper_t >::iterator entry = demuxer_wrappers.begin();

    while(entry != demuxer_wrappers.end())
    {
        score = (entry)->probe(&(*entry),dem_ctx);
        if(score == 1)
            break;
        entry ++;
    }
    if(entry == demuxer_wrappers.end())
        return -1;
    dem_ctx->demuxer = &(*entry);
    dt_info(TAG,"SELECT DEMUXER:%s \n", entry->name);
    return 0;
}

static void dump_media_info (dt_media_info_t * info)
{
    dt_info (TAG, "|====================MEDIA INFO====================|\n", info->file_name);
    dt_info (TAG, "|file_name:%s\n", info->file_name);
    dt_info (TAG, "|file_size:%d \n", info->file_size);
    dt_info (TAG, "|duration:%d bitrate:%d\n", info->duration, info->bit_rate);
    dt_info (TAG, "|has video:%d has audio:%d has sub:%d\n", info->has_video, info->has_audio, info->has_sub);
    dt_info (TAG, "|video stream info,num:%d\n", info->vst_num);
    int i = 0;
    for (i = 0; i < info->vst_num; i++)
    {
        dt_info (TAG, "|video stream:%d index:%d id:%d\n", i, info->vstreams[i]->index, info->vstreams[i]->id);
        dt_info (TAG, "|bitrate:%d width:%d height:%d duration:%lld \n", info->vstreams[i]->bit_rate, info->vstreams[i]->width, info->vstreams[i]->height, info->vstreams[i]->duration);
    }
    dt_info (TAG, "|audio stream info,num:%d\n", info->ast_num);
    for (i = 0; i < info->ast_num; i++)
    {
        dt_info (TAG, "|audio stream:%d index:%d id:%d\n", i, info->astreams[i]->index, info->astreams[i]->id);
        dt_info (TAG, "|bitrate:%d sample_rate:%d channels:%d bps:%d duration:%lld \n", info->astreams[i]->bit_rate, info->astreams[i]->sample_rate, info->astreams[i]->channels, info->astreams[i]->bps, info->astreams[i]->duration);
    }

    dt_info (TAG, "|subtitle stream num:%d\n", info->sst_num);
    dt_info (TAG, "|================================================|\n", info->file_name);
}

dtdemuxer_context::dtdemuxer_context(dtdemuxer_para_t& _para)
{
	para.file_name = _para.file_name;
	demuxer = nullptr;
	stream_ext = nullptr;
	parent = nullptr;
	probe_buf = nullptr;
}

int dtdemuxer_context::demuxer_open ()
{
    int ret = 0;
    /* open stream */
    dtstream_para_t para;
    para.stream_name = this->para.file_name;
	stream_ext = open_stream_module();
	ret = stream_ext->open(&para);
    if(ret != DTERROR_NONE)
    {
        dt_error (TAG, "stream open failed \n");
        return -1;
    }
   
    char value[512];
    int probe_enable = 0;
    int probe_size = PROBE_BUF_SIZE;
    if(GetEnv("DEMUXER","demuxer.probe",value) > 0)
    {
        probe_enable = atoi(value);
        dt_info(TAG,"probe enable:%d \n",probe_enable);
    }
    else
        dt_info(TAG,"probe enable not set, use default:%d \n",probe_enable);

    if(GetEnv("DEMUXER","demuxer.probesize",value) > 0)
    {
        probe_size = atoi(value);
        dt_info(TAG,"probe size:%d \n",probe_size);
    }
    else
        dt_info(TAG,"probe size not set, use default:%d \n",probe_size);


    if(probe_enable)
    {
		int64_t old_pos = stream_ext->tell();
        dt_info(TAG,"old:%lld \n",old_pos);
		probe_buf = new dt_buffer;
        ret = probe_buf->buf_init(PROBE_BUF_SIZE);
        if(ret < 0)
            return -1; 
		dt_info(TAG," buf init ok \n");
		ret = stream_ext->read(this->probe_buf->data,probe_size);
        if(ret <= 0)
            return -1;
        this->probe_buf->level = ret;
		ret = stream_ext->seek(old_pos,SEEK_SET);
        dt_info(TAG,"seek back to:%lld ret:%d \n",old_pos,ret);
    }

    /* select demuxer */
    if (demuxer_select (this) == -1)
    {
        dt_error (TAG, "select demuxer failed \n");
        return -1;
    }
    demuxer_wrapper_t *wrapper = this->demuxer;
    ret = wrapper->open (wrapper);
    if (ret < 0)
    {
        dt_error (TAG, "demuxer open failed\n");
        return -1;
    }
    dt_info (TAG, "demuxer open ok\n");
    dt_media_info_t *info = &(this->media_info);
    wrapper->setup_info (wrapper, info);
    dump_media_info (info);
    dt_info (TAG, "demuxer setup info ok\n");
    return 0;
}

int dtdemuxer_context::demuxer_read_frame (dt_av_frame_t * frame)
{
    demuxer_wrapper_t *wrapper = this->demuxer;
    return wrapper->read_frame (wrapper, frame);
}

int dtdemuxer_context::demuxer_seekto (int timestamp)
{
    demuxer_wrapper_t *wrapper = this->demuxer;
    return wrapper->seek_frame (wrapper, timestamp);
}

int dtdemuxer_context::demuxer_close ()
{
    int i = 0;
    demuxer_wrapper_t *wrapper = this->demuxer;
    wrapper->close (wrapper);
    /*free media info */
    dt_media_info_t *info = &(this->media_info);
    if (info->has_audio)
        for (i = 0; i < info->ast_num; i++)
        {
            if (info->astreams[i] == NULL)
                continue;
            if (info->astreams[i]->extradata_size)
                free (info->astreams[i]->extradata);
            free (info->astreams[i]);
            info->astreams[i] = NULL;
        }
    if (info->has_video)
        for (i = 0; i < info->vst_num; i++)
        {
            if (info->vstreams[i] == NULL)
                continue;
            if (info->vstreams[i]->extradata_size)
                free (info->vstreams[i]->extradata);
            free (info->vstreams[i]);
            info->vstreams[i] = NULL;
        }
    if (info->has_sub)
        for (i = 0; i < info->sst_num; i++)
        {
            if (info->sstreams[i] == NULL)
                continue;
            if (info->sstreams[i]->extradata_size)
                free (info->sstreams[i]->extradata);
            free (info->sstreams[i]);
            info->sstreams[i] = NULL;
        }
    /* release probe buf */
	if(probe_buf)
	{
		probe_buf->buf_release();
		delete(probe_buf);
	}
    /* close stream */
	stream_ext->close();
    return 0;
}
