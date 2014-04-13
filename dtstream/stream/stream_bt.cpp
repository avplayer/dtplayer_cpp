#include "dtstream.h"
#include "dt_error.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/lazy_entry.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/file_storage.hpp"
#include <boost/filesystem/operations.hpp>

#define TAG "STREAM-BT"

#ifndef AVSEEK_SIZE
#define AVSEEK_SIZE 0x10000
#endif

typedef struct{
    int fd;
}bt_ctx_t;

static int stream_bt_open (stream_wrapper_t * wrapper,char *stream_name)
{
	using namespace libtorrent;
	using namespace boost::filesystem;
	
	bt_ctx_t *ctx = (bt_ctx_t *)malloc(sizeof(*ctx));
	std::string filename(stream_name);
	int size = file_size(filename);
	if (size > 10 * 1000000)
	{
		std::cerr << "file too big (" << size << "), aborting\n";
		return 1;
	}
	std::vector<char> buf(size);
	std::ifstream(filename, std::ios_base::binary).read(&buf[0], size);
	lazy_entry e;
	int ret = lazy_bdecode(&buf[0], &buf[0] + buf.size(), e);

	if (ret != 0)
	{
		std::cerr << "invalid bencoding: " << ret << std::endl;
		return 1;
	}

	std::cout << "\n\n----- raw info -----\n\n";
	std::cout << print_entry(e) << std::endl;

	error_code ec;
	torrent_info t(e, ec);
	if (ec)
	{
		std::cout << ec.message() << std::endl;
		return 1;
	}

	// print info about torrent
	std::cout << "\n\n----- torrent file info -----\n\n";
	std::cout << "nodes:\n";
	typedef std::vector<std::pair<std::string, int> > node_vec;
	node_vec const& nodes = t.nodes();
	for (node_vec::const_iterator i = nodes.begin(), end(nodes.end());
		i != end; ++i)
	{
		std::cout << i->first << ":" << i->second << "\n";
	}
	std::cout << "trackers:\n";
#if 0
	for (std::vector<announce_entry>::const_iterator i = t.trackers().begin();
		i != t.trackers().end(); ++i)
	{
		std::cout << i->tier << ": " << i->url << "\n";
	}

	std::cout << "number of pieces: " << t.num_pieces() << "\n";
	std::cout << "piece length: " << t.piece_length() << "\n";
	char ih[41];
	to_hex((char const*)&t.info_hash()[0], 20, ih);
	std::cout << "info hash: " << ih << "\n";
	std::cout << "comment: " << t.comment() << "\n";
	std::cout << "created by: " << t.creator() << "\n";
	std::cout << "magnet link: " << make_magnet_uri(t) << "\n";
	std::cout << "name: " << t.name() << "\n";
	std::cout << "files:\n";
	int index = 0;
	for (torrent_info::file_iterator i = t.begin_files();
		i != t.end_files(); ++i, ++index)
	{
		int first = t.map_file(index, 0, 1).piece;
		int last = t.map_file(index, i->size - 1, 1).piece;
		std::cout << "  " << std::setw(11) << i->size
			<< " "
			<< (i->pad_file?'p':'-')
			<< (i->executable_attribute?'x':'-')
			<< (i->hidden_attribute?'h':'-')
			<< " "
			<< i->path.string() << "[ " << first << ", "
			<< last << " ]\n";
	}
#endif
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
