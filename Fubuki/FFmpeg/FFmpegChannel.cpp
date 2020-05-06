#include "FFmpegChannel.h"

using namespace std;
using namespace OnePunchMan;

const int FFmpegChannel::DestinationWidth = 1920;
const int FFmpegChannel::DestinationHeight = 1080;

FFmpegChannel::FFmpegChannel(const string& inputUrl, const string& outputUrl, bool debug)
	:ThreadObject("decode"),
	_inputUrl(inputUrl), _inputFormat(NULL), _inputStream(NULL), _inputVideoIndex(-1)
	, _outputUrl(outputUrl), _outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL)
	, _debug(debug), _options(NULL), _channelStatus(ChannelStatus::None), _sourceWidth(0), _sourceHeight(0)
	, _decodeContext(NULL), _yuvFrame(NULL), _bgrFrame(NULL), _bgrBuffer(NULL), _bgrSwsContext(NULL)
	, _lastPacketIndex(0), _packetSpan(0)
{
	if (inputUrl.size() >= 4 && inputUrl.substr(0,4).compare("rtsp") == 0)
	{
		av_dict_set(&_options, "rtsp_transport", "tcp", 0);
		av_dict_set(&_options, "stimeout", "5000000", 0);
	}
}

void FFmpegChannel::InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
	LogPool::Information(LogEvent::Decode, "init ffmpeg sdk");
}

void FFmpegChannel::UninitFFmpeg()
{
	avformat_network_deinit();
	LogPool::Information(LogEvent::Decode, "uninit video sdk");
}

const string& FFmpegChannel::InputUrl() const
{
	return _inputUrl;
}

ChannelStatus FFmpegChannel::Status()
{
	return _channelStatus;
}

int FFmpegChannel::SourceWidth() const
{
	return _sourceWidth;
}

int FFmpegChannel::SourceHeight() const
{
	return _sourceHeight;
}

int FFmpegChannel::PacketSpan()
{
	return _packetSpan;
}

ChannelStatus FFmpegChannel::Init()
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
	_sourceWidth = _inputStream->codecpar->width;
	_sourceHeight = _inputStream->codecpar->height;
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
		LogPool::Information(LogEvent::Decode, "init frame channel success", _inputUrl, _outputUrl, _debug);
		_channelStatus = ChannelStatus::Normal;
	}
	else
	{
		_channelStatus = ChannelStatus::DecoderError;
	}
	return _channelStatus;
}

void FFmpegChannel::Uninit()
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

bool FFmpegChannel::InitDecoder()
{
	if (_inputStream->codecpar->codec_id != AVCodecID::AV_CODEC_ID_H264)
	{
		LogPool::Warning(LogEvent::Decode, "codec is not h264", _inputUrl);
		return false;
	}

	AVCodec* decode = avcodec_find_decoder(_inputStream->codecpar->codec_id);
	if (decode == NULL) {
		LogPool::Warning(LogEvent::Decode, "avcodec_find_decoder error", _inputUrl);
		return false;
	}
	_decodeContext = avcodec_alloc_context3(decode);
	if (avcodec_parameters_to_context(_decodeContext, _inputStream->codecpar) < 0) {
		LogPool::Warning(LogEvent::Decode, "avcodec_parameters_to_context error", _inputUrl);
		return false;
	}

	if (avcodec_open2(_decodeContext, decode, NULL) < 0) {//打开解码器
		LogPool::Warning(LogEvent::Decode, "avcodec_open2 error", _inputUrl);
		return false;
	}

	//初始化帧
	_yuvFrame = av_frame_alloc();

	//yuv转rgb
	_bgrFrame = av_frame_alloc();
	_bgrBuffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_BGR24, DestinationWidth, DestinationHeight, 1));
	if (av_image_fill_arrays(_bgrFrame->data, _bgrFrame->linesize, _bgrBuffer, AV_PIX_FMT_BGR24, DestinationWidth, DestinationHeight, 1) < 0)
	{
		LogPool::Warning(LogEvent::Decode, "av_image_fill_arrays error");
		return false;
	}
	_bgrSwsContext = sws_getContext(_decodeContext->width, _decodeContext->height, _decodeContext->pix_fmt, DestinationWidth, DestinationHeight, AV_PIX_FMT_BGR24,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (_bgrSwsContext == NULL)
	{
		LogPool::Warning(LogEvent::Decode, "sws_getContext error", _inputUrl);
		return false;
	}
	return true;
}

void FFmpegChannel::UninitDecoder()
{
	if (_bgrSwsContext != NULL)
	{
		sws_freeContext(_bgrSwsContext);
	}
	if (_decodeContext != NULL) {
		avcodec_free_context(&_decodeContext);
	}
	if (_bgrFrame != NULL)
	{
		av_frame_free(&_bgrFrame);
	}
	if (_yuvFrame != NULL)
	{
		av_frame_free(&_yuvFrame);
	}
	if (_bgrBuffer != NULL)
	{
		av_free(_bgrBuffer);
	}
}

DecodeResult FFmpegChannel::Decode(const AVPacket* packet, int packetIndex)
{
	if (avcodec_send_packet(_decodeContext, packet) == 0)
	{
		while (true)
		{
			int resultReceive = avcodec_receive_frame(_decodeContext, _yuvFrame);
			if (resultReceive == AVERROR(EAGAIN) || resultReceive == AVERROR_EOF)
			{
				break;
			}
			else if (resultReceive < 0)
			{
				LogPool::Warning(LogEvent::Decode, "receive error", _inputUrl);
				return DecodeResult::Error;
			}
			else if (resultReceive >= 0)
			{
				if (sws_scale(_bgrSwsContext, _yuvFrame->data,
					_yuvFrame->linesize, 0, _decodeContext->height,
					_bgrFrame->data, _bgrFrame->linesize) != 0)
				{
					//_bgrHandler.HandleFrame(_bgrFrame->data[0], _decodeContext->width, _decodeContext->height, packetIndex);
				}
			}
		}
	}
	else
	{
		LogPool::Warning(LogEvent::Decode, "send error", _inputUrl);
		return DecodeResult::Error;
	}
	return DecodeResult::Handle;
}

void FFmpegChannel::StartCore()
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
			if (_debug)
			{
				LogPool::Information(LogEvent::Decode, "read eof", _inputUrl);
				break;
			}
			else
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
		}
		else if (result != 0)
		{
			LogPool::Error(LogEvent::Decode, "read error", result, _inputUrl);
			_channelStatus = ChannelStatus::ReadError;
			break;
		}
		if (packet.stream_index == _inputVideoIndex)
		{
			long long start = DateTime::UtcNowTimeStamp();
			packetIndex += 1;
			DecodeResult result = Decode(&packet, packetIndex);
			if (result == DecodeResult::Error)
			{
				_channelStatus = ChannelStatus::DecodeError;
				break;
			}
			else if (result == DecodeResult::Handle)
			{
				//LogPool::Debug(LogEvent::Decode, "handle packet", _lastPacketIndex, packetIndex);
				_packetSpan = packetIndex - _lastPacketIndex;
				_lastPacketIndex = packetIndex;
			}

			if (_outputFormat != NULL)
			{
				packet.pts = duration * loopCount + av_rescale_q_rnd(packet.pts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				packet.dts = duration * loopCount + av_rescale_q_rnd(packet.dts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				packet.duration = av_rescale_q(packet.duration, _inputStream->time_base, _outputStream->time_base);
				packet.pos = -1;
				if (av_write_frame(_outputFormat, &packet) != 0)
				{
					//LogPool::Debug(LogEvent::Decode, "write frame",_outputUrl, packetIndex);
				}
			}
			long long end = DateTime::UtcNowTimeStamp();
			long long sleepTime = frameSpan - (end - start);
			if (sleepTime > 0)
			{
				this_thread::sleep_for(chrono::milliseconds(sleepTime));
			}
			//LogPool::Debug(LogEvent::Decode, "get packet", packetIndex, sleepTime,static_cast<int>(result));
		}
		av_packet_unref(&packet);
	}
	Uninit();
}

