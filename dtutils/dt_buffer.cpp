#include "dt_buffer.h"
#include "dt_log.h"

#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#define TAG "DT-BUF"
/*buffer ops*/

dt_buffer::dt_buffer()
{
	data = nullptr;
	size = 0;
}

int dt_buffer::buf_init (int size)
{
	dt_buffer_t *dbt = this;
	uint8_t *buffer = new uint8_t[size];
    //uint8_t *buffer = (uint8_t *) malloc (size);
    if (!buffer)
    {
        dbt->data = NULL;
        return -1;
    }
    dbt->data = buffer;
    dbt->size = size;
    dbt->level = 0;
    dbt->rd_ptr = dbt->wr_ptr = dbt->data;
    dt_info (TAG, "DTBUF INIT OK\n");
    return 0;
}

int dt_buffer::buf_reinit ()
{
	dt_buffer_t *dbt = this;
	mutex.lock();
    dbt->level = 0;
    dbt->rd_ptr = dbt->wr_ptr = dbt->data;
	mutex.unlock();
    return 0;
}

int dt_buffer::buf_release ()
{
	dt_buffer_t *dbt = this;
	mutex.lock();
    if (dbt->data)
		delete(dbt->data);
    dbt->size = 0;
	mutex.unlock();
    return 0;
}

static int buf_empty (dt_buffer_t * dbt)
{
    //no need to lock, will lock uplevel
    int ret = -1;
    if (dbt->level == 0)
        ret = 1;
    else
        ret = 0;
    return ret;
}

static int buf_full (dt_buffer_t * dbt)
{
    //no need to lock, will lock uplevel
    int ret = -1;
    if (dbt->level == dbt->size)
        ret = 1;
    else
        ret = 0;
    return ret;
}

int dt_buffer::buf_space ()
{
	dt_buffer_t *dbt = this;
    int space = dbt->size - dbt->level;
    return space;
}

int dt_buffer::buf_level ()
{
	dt_buffer_t *dbt = this;
    int lev = dbt->level;
    return lev;
}

int dt_buffer::buf_get (uint8_t * out, int size)
{
	dt_buffer_t *dbt = this;
	mutex.lock();
    int len = -1;
    len = buf_empty (dbt);
    if (len == 1)
    {
        len = 0;
        goto QUIT;              //get nothing
    }

    len = MIN (dbt->level, size);
    if (dbt->wr_ptr > dbt->rd_ptr)
    {
        memcpy (out, dbt->rd_ptr, len);
        dbt->rd_ptr += len;
        dbt->level -= len;
        goto QUIT;

    }
    else if (len <= (int)(dbt->data + dbt->size - dbt->rd_ptr))
    {
        memcpy (out, dbt->rd_ptr, len);
        dbt->rd_ptr += len;
        dbt->level -= len;
        goto QUIT;

    }
    else
    {
        int tail_len = (int)(dbt->data + dbt->size - dbt->rd_ptr);
        memcpy (out, dbt->rd_ptr, tail_len);
        memcpy (out + tail_len, dbt->data, len - tail_len);
        dbt->rd_ptr = dbt->data + len - tail_len;
        dbt->level -= len;
        goto QUIT;
    }
  QUIT:
	mutex.unlock();
    return len;
}

int dt_buffer::buf_put (uint8_t * in, int size)
{
	dt_buffer_t *dbt = this;
	mutex.lock();
    int len = buf_full (dbt);
    if (len == 1)
    {
        len = 0;
        goto QUIT;              // no space to write
    }

    len = MIN (dbt->size - dbt->level, size);
    if (dbt->wr_ptr < dbt->rd_ptr)
    {
        memcpy (dbt->wr_ptr, in, len);
        dbt->wr_ptr += len;
        dbt->level += len;
        goto QUIT;

    }
    else if (len <= (int)(dbt->data + dbt->size - dbt->wr_ptr))
    {
        memcpy (dbt->wr_ptr, in, len);
        dbt->wr_ptr += len;
        dbt->level += len;
        goto QUIT;

    }
    else
    {
        int tail_len = (int)(dbt->data + dbt->size - dbt->wr_ptr);
        memcpy (dbt->wr_ptr, in, tail_len);
        memcpy (dbt->data, in + tail_len, len - tail_len);
        dbt->wr_ptr = dbt->data + len - tail_len;
        dbt->level += len;
        goto QUIT;
    }
  QUIT:
	mutex.unlock();
    return len;
}
