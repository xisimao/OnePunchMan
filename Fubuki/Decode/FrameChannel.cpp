#include "FrameChannel.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;

FrameChannel::FrameChannel(const string& inputUrl, const string& outputUrl,bool loop)
	:ThreadObject("decode"),
	_inputUrl(inputUrl), _inputFormat(NULL), _inputStream(NULL), _inputVideoIndex(-1)
	, _outputUrl(outputUrl), _outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL)
	, _options(NULL), _loop(loop), _channelStatus(ChannelStatus::Normal)
{
	if (inputUrl.size() >= 4&&inputUrl.substr(4).compare("rtsp")==0)
	{
		av_dict_set(&_options, "rtsp_transport", "tcp", 0);
		av_dict_set(&_options, "stimeout", "5000000", 0);
	}
}

void FrameChannel::InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
	LogPool::Information(LogEvent::Decode, "init ffmpeg sdk");
}

void FrameChannel::UninitFFmpeg()
{
	avformat_network_deinit();
	LogPool::Information(LogEvent::Decode, "uninit video sdk");
}

ChannelStatus FrameChannel::Status()
{
	return _channelStatus;
}

ChannelStatus FrameChannel::Init()
{
	_inputFormat = avformat_alloc_context();
	if (avformat_open_input(&_inputFormat, _inputUrl.c_str(), NULL, &_options) != 0) {
		LogPool::Error(LogEvent::Decode, "avformat_open_input", _inputUrl);
		_channelStatus = ChannelStatus::InputError;
		return _channelStatus;
	}

	if (avformat_find_stream_info(_inputFormat, NULL) < 0) {
		LogPool::Error(LogEvent::Decode, "avformat_find_stream_info", _inputUrl);
		_channelStatus = ChannelStatus::InputError;
		return _channelStatus;
	}

	for (unsigned int i = 0; i < _inputFormat->nb_streams; i++) {
		if (_inputFormat->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			_inputVideoIndex = i;
			break;
		}
	}
	if (_inputVideoIndex == -1) {
		LogPool::Error(LogEvent::Decode, "not found video index", _inputUrl);
		_channelStatus = ChannelStatus::InputError;
		return _channelStatus;
	}

	_inputStream = _inputFormat->streams[_inputVideoIndex];

	if (!_outputUrl.empty())
	{
		avformat_alloc_output_context2(&_outputFormat, NULL, "flv", _outputUrl.c_str());
		if (_outputFormat == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_alloc_output_context2", _outputUrl);
			_channelStatus = ChannelStatus::OutputError;
			return _channelStatus;
		}

		AVCodec* decode = avcodec_find_decoder(_inputStream->codecpar->codec_id);
		if (decode == NULL) {
			LogPool::Error(LogEvent::Decode, "avcodec_find_decoder", _inputUrl);
			_channelStatus = ChannelStatus::InputError;
			return _channelStatus;
		}

		_outputStream = avformat_new_stream(_outputFormat, decode);
		if (_outputStream == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_new_stream", _outputUrl);
			_channelStatus = ChannelStatus::OutputError;
			return _channelStatus;
		}

		_outputCodec = avcodec_alloc_context3(decode);
		if (avcodec_parameters_to_context(_outputCodec, _inputStream->codecpar) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", _inputUrl);
			_channelStatus = ChannelStatus::InputError;
			return _channelStatus;
		}
		_outputCodec->codec_tag = 0;
		if (_outputFormat->oformat->flags & AVFMT_GLOBALHEADER)
		{
			_outputCodec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

		if (avcodec_parameters_from_context(_outputStream->codecpar, _outputCodec) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", _outputUrl);
			_channelStatus = ChannelStatus::OutputError;
			return _channelStatus;
		}
		if (!(_outputFormat->oformat->flags & AVFMT_NOFILE))
		{
			if (avio_open(&_outputFormat->pb, _outputUrl.c_str(), AVIO_FLAG_WRITE))
			{
				LogPool::Error(LogEvent::Decode, "avio_open", _outputUrl);
				_channelStatus = ChannelStatus::OutputError;
				return _channelStatus;
			}
		}
		if (avformat_write_header(_outputFormat, NULL) < 0) {
			LogPool::Error(LogEvent::Decode, "avformat_write_header", _outputUrl);
			_channelStatus = ChannelStatus::OutputError;
			return _channelStatus;
		}
	}

	if (InitDecoder()) {
		LogPool::Information(LogEvent::Decode, "init frame channel success", _inputUrl, _outputUrl, _loop);
		_channelStatus = ChannelStatus::Normal;
	}
	else
	{
		_channelStatus = ChannelStatus::DecoderError;
	}
	return _channelStatus;
}

void FrameChannel::Uninit()
{
	UninitDecoder();
	if (_outputFormat != NULL)
	{
		av_write_trailer(_outputFormat);
		if (_outputFormat && !(_outputFormat->flags & AVFMT_NOFILE))
			avio_close(_outputFormat->pb);
		avcodec_close(_outputCodec);
		avformat_free_context(_outputFormat);
	}

	if (_inputFormat != NULL) {
		avformat_close_input(&_inputFormat);
	}
}

void FrameChannel::StartCore()
{
	if (Init() != ChannelStatus::Normal)
	{
		return;
	}

	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	long long frameSpan = (1000 / _inputStream->avg_frame_rate.num);
	long long duration = _inputStream->duration;
	int packetIndex = 0;
	int loopCount = 0;
	while (!_cancelled)
	{
		int result = av_read_frame(_inputFormat, &packet);
		if (result == AVERROR_EOF)
		{
			Decode(NULL, packetIndex);
			if (_loop)
			{
				Uninit();
				if (Init() != ChannelStatus::Normal)
				{
					return;
				}
				loopCount += 1;
				packetIndex = 0;
				continue;
			}
			else
			{
				LogPool::Information(LogEvent::Decode, "read eof", _inputUrl);
				break;
			}
		}
		else if (result != 0)
		{
			LogPool::Error(LogEvent::Decode, "read error", result, _inputUrl);
			_channelStatus = ChannelStatus::ReadError;
			break;
		}
		if (packet.stream_index == _inputVideoIndex) 
		{
			long long start = DateTime::UtcTimeStamp();
			packetIndex += 1;
			if (!Decode(&packet, packetIndex))
			{
				_channelStatus = ChannelStatus::DecodeError;
				break;
			}	
			if (_outputFormat != NULL)
			{
				packet.pts = duration * loopCount + av_rescale_q_rnd(packet.pts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				packet.dts = duration * loopCount + av_rescale_q_rnd(packet.dts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				packet.duration = av_rescale_q(packet.duration, _inputStream->time_base, _outputStream->time_base);
				packet.pos = -1;
				av_interleaved_write_frame(_outputFormat, &packet);
			}
			long long end = DateTime::UtcTimeStamp();
			long long sleepTime = frameSpan - (end - start);
			if (sleepTime > 0)
			{
				this_thread::sleep_for(chrono::milliseconds(sleepTime));
			}
			//LogPool::Debug(LogEvent::Decode, "get packet", packetIndex, end-start);
		}
		av_packet_unref(&packet);
	}
	Uninit();
	_channelStatus = ChannelStatus::End;
}

