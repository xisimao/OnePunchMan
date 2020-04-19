#include "FrameChannel.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;

FrameChannel::FrameChannel()
	:ThreadObject("decode"),
	_url(), _formatContext(NULL), _videoIndex(-1)
	,_options(NULL),_sync(false), _loop(false)
{

}

void FrameChannel::InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
	LogPool::Warning(LogEvent::Decode, "init video sdk");
}

void FrameChannel::UninitFFmpeg()
{
	avformat_network_deinit();
	LogPool::Warning(LogEvent::Decode, "uninit video sdk");
}

int FrameChannel::InitRtsp(const string& url)
{
	av_dict_set(&_options, "rtsp_transport", "tcp", 0);
	av_dict_set(&_options, "stimeout", "5000000", 0);
	_url = url;
	_sync = true;
	_loop = false;
	return Init();
}

int FrameChannel::InitFile(const string& url, bool sync, bool loop)
{
	_url = url;
	_sync = sync;
	_loop = loop;
	return Init();
}

int FrameChannel::Init()
{
	_formatContext = avformat_alloc_context();
	if (avformat_open_input(&_formatContext, _url.c_str(), NULL, &_options) != 0) {
		LogPool::Error(LogEvent::Decode, "avformat_open_input error", _url);
		return 1;
	}

	if (avformat_find_stream_info(_formatContext, NULL) < 0) {
		LogPool::Error(LogEvent::Decode, "avformat_find_stream_info error", _url);
		return 2;
	}
	for (unsigned int i = 0; i < _formatContext->nb_streams; i++) {
		if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			_videoIndex = i;
			break;
		}
	}
	if (_videoIndex == -1) {
		LogPool::Error(LogEvent::Decode, "not found video index", _url);
		return 3;
	}
	int result = InitCore();
	if (result == 0) {
		LogPool::Information(LogEvent::Decode, "init frame channel success", _url, _sync, _loop);
	}
	return result;
}

void FrameChannel::Uninit()
{
	UninitCore();
	if (_formatContext != NULL) {
		avformat_close_input(&_formatContext);
	}
}

void FrameChannel::StartCore()
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	long long frameSpan = (1000 / _formatContext->streams[_videoIndex]->avg_frame_rate.num);
	int packetIndex = 0;
	while (!_cancelled)
	{
		int result = av_read_frame(_formatContext, &packet);
		if (result == AVERROR_EOF)
		{
			Decode(NULL, packetIndex);
			if (_loop)
			{
				Uninit();
				Init();
				packetIndex = 0;
				continue;
			}
			else
			{
				LogPool::Information(LogEvent::Decode, "read eof", _url);
				break;
			}
		}
		else if (result != 0)
		{
			LogPool::Warning(LogEvent::Decode, "read error", result, _url);
			break;
		}
		if (packet.stream_index == _videoIndex) 
		{
			packetIndex += 1;
			long long start = DateTime::TimeStamp();
			if (!Decode(&packet, packetIndex))
			{
				break;
			}
			long long end = DateTime::TimeStamp();
			long long sleepTime = frameSpan - (end - start);
			if (sleepTime > 0)
			{
				this_thread::sleep_for(chrono::milliseconds(sleepTime));
			}
			//LogPool::Debug(LogEvent::Decode, "get packet", _url, packetIndex, sleepTime);
		}
		av_packet_unref(&packet);
	}
	Uninit();
}

