#define DT_MSG_DELIM "---------------------------------------------------"

#include "dt_buffer.h"
#include "dt_event.h"
#include "dt_event_def.h"
#include "dt_lock.h"
#include "dt_log.h"
#include "dt_time.h"
#include "dt_ini.h"
#include "dt_rw.h"


static const char *version_tags = "";

void version_info (void)
{
    dt_info (version_tags, DT_MSG_DELIM "\n");
    dt_info (version_tags, "  dtmedia version 0.9\n");
    dt_info (version_tags, "  based on FFMPEG, built on " __DATE__ " " __TIME__ "\n");
    dt_info (version_tags, "  GCC: " __VERSION__ "\n");
    dt_info (version_tags, DT_MSG_DELIM "\n");
}

void show_usage ()
{
    dt_info (version_tags, DT_MSG_DELIM "\n");
    dt_info (version_tags, " Usage: dtplayer [options] [url|path/]filename \n\n");
    dt_info (version_tags, " Basic options: \n");
    dt_info (version_tags, " -w <width>  set video width \n");
    dt_info (version_tags, " -h <height> set video height \n");
    dt_info (version_tags, " -noaudio    disable audio \n");
    dt_info (version_tags, " -novideo    disable video \n");
    dt_info (version_tags, DT_MSG_DELIM "\n");
}
