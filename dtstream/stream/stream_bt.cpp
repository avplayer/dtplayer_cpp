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
#include <boost/lambda/lambda.hpp>


#include "libtorrent/utf8.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/storage.hpp"


#define TAG "STREAM-BT"

#ifndef AVSEEK_SIZE
#define AVSEEK_SIZE 0x10000
#endif

using namespace libtorrent;
using namespace boost::filesystem;

// torrent中的视频信息.
struct video_file_info 
{
	video_file_info()
		: offset(-1)	// m_offset标识为-1, 当初始化时应该修正为base_offset.
	{}

	int index;						// id.
	std::string filename;			// 视频文件名.
	boost::uint64_t data_size;		// 视频大小.
	boost::uint64_t base_offset;	// 视频在torrent中的偏移.
	uint64_t offset;				// 数据访问的偏移, 相对于base_offset.
	int status;						// 当前播放状态.
};

typedef struct{
	// session下载对象.
	session m_session;

	// torrent handle.
	torrent_handle m_torrent_handle;
	
	// 当前播放的视频.
	video_file_info m_current_video;

	// torrent中所有视频信息.
	std::vector<video_file_info> m_videos;
}bt_ctx_t;

static int stream_bt_open (stream_wrapper_t * wrapper,char *stream_name)
{
	bt_ctx_t *ctx = new bt_ctx_t;
	stream_ctrl_t *info = &wrapper->info;
    memset(info,0,sizeof(*info));
	ctx->m_current_video.index = -1;
	
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
	//std::cout << print_entry(e) << std::endl;

	error_code ec;
	add_torrent_params p;
	p.save_path = "~/.dtplayer/data/";
	torrent_info *t = new torrent_info(e,ec);
	//torrent_info t(e, ec);
	if (ec)
	{
		std::cout << ec.message() << std::endl;
		return 1;
	}
	p.ti = t;

	// print info about torrent
	std::cout << "\n\n----- torrent file info -----\n\n";
	std::cout << "nodes:\n";
	typedef std::vector<std::pair<std::string, int> > node_vec;
	node_vec const& nodes = t->nodes();
	for (node_vec::const_iterator i = nodes.begin(), end(nodes.end());
		i != end; ++i)
	{
		std::cout << i->first << ":" << i->second << "\n";
	}
	std::cout << "trackers:\n";

	for (std::vector<announce_entry>::const_iterator i = t->trackers().begin();
		i != t->trackers().end(); ++i)
	{
		std::cout << i->tier << ": " << i->url << "\n";
	}

	std::cout << "number of pieces: " << t->num_pieces() << "\n";
	std::cout << "piece length: " << t->piece_length() << "\n";
	char ih[41];
	to_hex((char const*)&t->info_hash()[0], 20, ih);
	std::cout << "info hash: " << ih << "\n";
	std::cout << "comment: " << t->comment() << "\n";
	std::cout << "created by: " << t->creator() << "\n";
	std::cout << "magnet link: " << make_magnet_uri(*t) << "\n";
	std::cout << "name: " << t->name() << "\n";
	std::cout << "files:\n";
	int index = 0;
	for (torrent_info::file_iterator i = t->begin_files();
		i != t->end_files(); ++i, ++index)
	{
		int first = t->map_file(index, 0, 1).piece;
		int last = t->map_file(index, i->size - 1, 1).piece;
		std::cout << "  " << std::setw(11) << i->size
			<< " "
			<< (i->pad_file?'p':'-')
			<< (i->executable_attribute?'x':'-')
			<< (i->hidden_attribute?'h':'-')
			<< " "
			<< i->filename() << "[ " << first << ", "
 			//<< i->path.string() << "[ " << first << ", "
			<< last << " ]\n";
	}
	
	// 遍历视频文件.
	const file_storage &fs = p.ti->files();
	for (file_storage::iterator i = fs.begin();
		i != fs.end(); i++)
	{
		boost::filesystem::path p(convert_to_native(i->filename()));
		std::string ext = p.extension().string();
		if (ext == ".rmvb" ||
			ext == ".wmv" ||
			ext == ".avi" ||
			ext == ".mkv" ||
			ext == ".flv" ||
			ext == ".rm" ||
			ext == ".mp4" ||
			ext == ".3gp" ||
			ext == ".webm" ||
			ext == ".mpg")
		{
			video_file_info vfi;
			vfi.filename = convert_to_native(i->filename());
			vfi.base_offset = i->offset;
			vfi.offset = i->offset;
			vfi.data_size = i->size;
			vfi.index = index++;
			vfi.status = 0;
			std::cout<<"index:" << vfi.index
			         <<" -- name:" << vfi.filename
			         <<" -- base_offset:"<<vfi.base_offset
			         <<" -- offset:"<< vfi.offset
			         <<" -- data_size:"<< vfi.data_size
			         <<std::endl;
			// 当前视频默认置为第一个视频.
			if (ctx->m_current_video.index == -1)
				ctx->m_current_video = vfi;

			// 保存到视频列表中.
			ctx->m_videos.push_back(vfi);
		}
	}
	
	ctx->m_session.add_dht_router(std::make_pair(
		std::string("router.bittorrent.com"), 6881));
	ctx->m_session.add_dht_router(std::make_pair(
		std::string("router.utorrent.com"), 6881));
	ctx->m_session.add_dht_router(std::make_pair(
		std::string("router.bitcomet.com"), 6881));

	ctx->m_session.start_dht();

	//    m_session.load_asnum_db("GeoIPASNum.dat");
	//    m_session.load_country_db("GeoIP.dat");

	ctx->m_session.listen_on(std::make_pair(6881, 6889));

	// 设置缓冲.
	session_settings settings = ctx->m_session.settings();
	settings.use_read_cache = false;
	settings.disk_io_read_mode = session_settings::disable_os_cache;
	settings.broadcast_lsd = true;
	settings.allow_multiple_connections_per_ip = true;
	settings.local_service_announce_interval = 15;
	settings.min_announce_interval = 20;
	ctx->m_session.set_settings(settings);

	// 添加到session中.
	ctx->m_torrent_handle = ctx->m_session.add_torrent(p, ec);
	if (ec)
	{
		printf("%s\n", ec.message().c_str());
		return false;
	}

	ctx->m_torrent_handle.force_reannounce();

	// 自定义播放模式下载.
	//ctx->m_torrent_handle.set_user_defined_download(true);
	

	// 限制上传速率为测试.
	// ctx->m_session.set_upload_rate_limit(80 * 1024);
	// ctx->m_session.set_local_upload_rate_limit(80 * 1024);

	//info set
	wrapper->stream_priv = (void *)ctx;
	info->stream_size = ctx->m_current_video.data_size;
	info->is_stream = 1;
    info->seek_support = 1;
    info->cur_pos = 0;
	
	dt_info(TAG,"BT OPEN OK\n");
    return DTERROR_NONE;
}

static int stream_bt_read (stream_wrapper_t * wrapper,uint8_t *buf,int len)
{
	bt_ctx_t *ctx = (bt_ctx_t *)wrapper->stream_priv;
    stream_ctrl_t *info = &wrapper->info;

	dt_info(TAG,"read bt data, len:%d \n",len);

	// 必须保证read_data函数退出后, 才能destroy这个对象!!!
	int read_size = 0;

	// 读取数据越界.
	if (ctx->m_current_video.offset >= ctx->m_current_video.base_offset + ctx->m_current_video.data_size ||
		ctx->m_current_video.offset < ctx->m_current_video.base_offset)
	{
		dt_info(TAG,"OFFSET INVALID, offset:%llu base_offset:%lld data_size:%llu \n",ctx->m_current_video.offset,ctx->m_current_video.base_offset,ctx->m_current_video.data_size);
		return -1;
	}
	uint64_t &offset = ctx->m_current_video.offset;
	const torrent_info &tinfo = ctx->m_torrent_handle.get_torrent_info();
	
	// 计算偏移所在的片.
	int index = offset / tinfo.piece_length();
	BOOST_ASSERT(index >= 0 && index < tinfo.num_pieces());
	if (index >= tinfo.num_pieces() || index < 0)
	{
		dt_info(TAG,"INDEX INVALID\n");
		return -1;
	}
	torrent_status status = ctx->m_torrent_handle.status();
	if (!status.pieces.empty())
	{
		if (status.state != torrent_status::downloading &&
			status.state != torrent_status::finished &&
			status.state != torrent_status::seeding)
		{
			dt_info(TAG,"BT IS NOT DOWNLOADING \n");
			return 0;
		}

		// 设置下载位置.
		std::vector<int> pieces;

		pieces = ctx->m_torrent_handle.piece_priorities();
		std::for_each(pieces.begin(), pieces.end(), boost::lambda::_1 = 1);
		pieces[index] = 7;
		ctx->m_torrent_handle.prioritize_pieces(pieces);

		if (status.pieces.get_bit(index))
		{
			// 提交请求.
			ctx->m_torrent_handle.read_piece(index);
			dt_info(TAG,"READ ONE PIECE , INDEX:%d \n",index);
		
		}		
		return 0;

	}
	else
		dt_info(TAG,"READ ONE PIECE , But No data left\n",index);
	
	return -1;
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
