#ifndef DT_BUFFER_T
#define DT_BUFFER_T

#include "dt_macro.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <mutex>

typedef struct dt_buffer
{
    uint8_t *data;
    int size;
    int level;
    uint8_t *rd_ptr;
    uint8_t *wr_ptr;
	std::mutex mutex;
	
	dt_buffer();
	int buf_init (int size);
	int buf_reinit ();
	int buf_release ();
	int buf_space ();
	int buf_level ();
	int buf_get (uint8_t * out, int size);
	int buf_put (uint8_t * in, int size);
	
} dt_buffer_t;


#endif
