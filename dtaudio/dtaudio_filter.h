#ifndef DTAUDIO_FILTER_H
#define DTAUDIO_FILTER_H
typedef struct
{
    int filter;
} dtaudio_filter_t;

int audio_filter_init (dtaudio_filter_t*);
int audio_filter_start (dtaudio_filter_t*);
int audio_filter_stop (dtaudio_filter_t*);
int audio_filter_release (dtaudio_filter_t*);
#endif
