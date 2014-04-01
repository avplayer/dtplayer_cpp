#include "dt_packet_queue.h"

#include "unistd.h"

#define TAG "PORT-QUEUE"
/*packet queue operation*/
int packet_queue_init (dt_packet_queue_t * queue)
{
    if (!queue)
        return -1;
    //memset(queue,0,sizeof(queue));
    queue->first = NULL;
    queue->last = NULL;
    queue->size = 0;
    queue->nmutex = 0;
    return 0;
}

int packet_queue_put_frame (dt_packet_queue_t * queue, dt_av_frame_t * frame)
{
    if (!queue)
    {
        printf ("[%s:%d] queue==NULL \n", __FUNCTION__, __LINE__);
        return -1;
    }
    dt_packet_list_t *list;
    list = (dt_packet_list_t*) malloc (sizeof (dt_packet_list_t));
    if (!list)
        return -1;
    //list->frame=*frame;
    memcpy (&list->frame, frame, sizeof (dt_av_frame_t));
    list->next = NULL;
    if (!(queue->last))
        queue->first = list;
    else
        queue->last->next = list;
    queue->last = list;
    queue->nb_packets++;
    queue->size += frame->size;
    //queue->size+=frame->size+sizeof(*list);
    //printf("[%s:%d] packet_nb=%d size:%d \n",__FUNCTION__,__LINE__,queue->nb_packets,queue->size);
    return 0;
}

int packet_queue_put (dt_packet_queue_t * queue, dt_av_frame_t * frame)
{
    int ret;
    if (queue->nmutex)
        return -1;
    queue->nmutex = 1;
    if (frame->type == 0 && queue->size >= QUEUE_MAX_VBUF_SIZE)
    {
        dt_debug (TAG, "[%s:%d]type:%d(0 video 1 audio) packet queue exceed size\n", __FUNCTION__, __LINE__, frame->type);
        queue->nmutex = 0;
        return -2;
    }

    if (frame->type == 1 && queue->size >= QUEUE_MAX_ABUF_SIZE)
    {
        dt_debug (TAG, "[%s:%d]type:%d(0 video 1 audio) packet queue exceed size\n", __FUNCTION__, __LINE__, frame->type);
        queue->nmutex = 0;
        return -2;
    }
    ret = packet_queue_put_frame (queue, frame);
    queue->nmutex = 0;
    //printf("[%s:%d] packet queue in ok\n",__FUNCTION__,__LINE__);
    return ret;
}

int packet_queue_get_frame (dt_packet_queue_t * queue, dt_av_frame_t * frame)
{
    if (queue->nb_packets == 0)
    {
//        printf("[%s:%d] No packet left\n",__FUNCTION__,__LINE__);
        return -1;
    }
    dt_packet_list_t *list;
    list = queue->first;
    if (list)
    {
        queue->first = list->next;
        if (!queue->first)
            queue->last = NULL;
        queue->nb_packets--;
        *frame = (list->frame);
        queue->size -= frame->size;
        //queue->size-=frame->size+sizeof(*list);
        list->frame.data = NULL;
        free (list);
        //printf("[%s:%d] queue get frame ok\n",__FUNCTION__,__LINE__);
        return 0;
    }
    return -1;
}

int packet_queue_get (dt_packet_queue_t * queue, dt_av_frame_t * frame)
{
    int ret;
    if (queue->nmutex)
        return -1;
    queue->nmutex = 1;
    ret = packet_queue_get_frame (queue, frame);
    queue->nmutex = 0;
    return ret;
}

int packet_queue_size (dt_packet_queue_t * queue)
{
    return queue->nb_packets;
}

int packet_queue_data_size (dt_packet_queue_t * queue)
{
    return queue->size;
}

int packet_queue_flush (dt_packet_queue_t * queue)
{
    dt_packet_list_t *list1, *list2;
    while (queue->nmutex)
        usleep (1000);
    queue->nmutex = 1;
    for (list1 = queue->first; list1 != NULL; list1 = list2)
    {
        list2 = list1->next;
        //dt_debug(TAG,"free queue, data addr:%p list1:%p size:%d\n",list1->frame.data,list1,list1->frame.size);
        free (list1->frame.data);
        //data malloc outside ,but free here if receive stop cmd
        //free(&list1->frame);//will free in free(list1) ops
        free (list1);
    }
    queue->last = NULL;
    queue->first = NULL;
    queue->nb_packets = 0;
    queue->size = 0;
    queue->nmutex = 0;
    return 0;
}

int packet_queue_release (dt_packet_queue_t * queue)
{
    return packet_queue_flush (queue);
}
