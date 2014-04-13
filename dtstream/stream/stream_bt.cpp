#include "dtstream.h"
#include "dt_error.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "libtorrent/extensions/metadata_transfer.hpp"
#include "libtorrent/extensions/ut_metadata.hpp"
#include "libtorrent/extensions/ut_pex.hpp"
#include "libtorrent/extensions/smart_ban.hpp"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/identify_client.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/ip_filter.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/bitfield.hpp"
#include "libtorrent/file.hpp"
#include "libtorrent/peer_info.hpp"
#include "libtorrent/socket_io.hpp" // print_address
#include "libtorrent/time.hpp"
#include "libtorrent/create_torrent.hpp"

#define TAG "STREAM-BT"

#ifndef AVSEEK_SIZE
#define AVSEEK_SIZE 0x10000
#endif

typedef struct{
    int fd;
    int64_t file_size;
}bt_ctx_t;

static int stream_bt_open (stream_wrapper_t * wrapper,char *stream_name)
{
    return DTERROR_NONE;
}

static int stream_bt_read (stream_wrapper_t * wrapper,uint8_t *buf,int len)
{
    return 0;
}

static int stream_bt_seek (stream_wrapper_t * wrapper, int64_t pos, int whence)
{
    return DTERROR_NONE;
}

static int stream_bt_close (stream_wrapper_t * wrapper)
{
    return 0;
}

stream_wrapper_t stream_bt(STREAM_BT, "BT", stream_bt_open, stream_bt_read, stream_bt_seek, stream_bt_close);
