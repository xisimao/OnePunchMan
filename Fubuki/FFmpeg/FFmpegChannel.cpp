#include "FFmpegChannel.h"

using namespace std;
using namespace OnePunchMan;

const int FFmpegChannel::ConnectSpan = 5;
const int FFmpegChannel::DestinationWidth = 1920;
const int FFmpegChannel::DestinationHeight = 1080;

FFmpegChannel::FFmpegChannel(const string& inputUrl, const string& outputUrl, bool debug)
	:ThreadObject("decode"),
	_inputUrl(inputUrl), _inputFormat(NULL), _inputStream(NULL), _inputVideoIndex(-1)
	, _outputUrl(outputUrl), _outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL)
	, _debug(debug), _options(NULL), _channelStatus(ChannelStatus::None), _sourceWidth(0), _sourceHeight(0)
	, _decodeContext(NULL), _yuvFrame(NULL), _bgrFrame(NULL), _bgrBuffer(NULL), _bgrSwsContext(NULL)
	, _lastframeIndex(0), _frameSpan(0)
{
	if (inputUrl.size() >= 4 && inputUrl.substr(0,4).compare("rtsp") == 0)
	{
		av_dict_set(&_options, "rtsp_transport", "tcp", 0);
		av_dict_set(&_options, "stimeout", StringEx::ToString(ConnectSpan*1000*1000).c_str(), 0);
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

int FFmpegChannel::FrameSpan()
{
	return _frameSpan;
}

bool FFmpegChannel::InitInput()
{
	if (_inputFormat == NULL)
	{
		_inputFormat = avformat_alloc_context();
		if (avformat_open_input(&_inputFormat, _inputUrl.c_str(), NULL, &_options) != 0) {
			LogPool::Error(LogEvent::Decode, "avformat_open_input", _inputUrl);
			UninitInput();
			return false;
		}

		if (avformat_find_stream_info(_inputFormat, NULL) < 0) {
			LogPool::Error(LogEvent::Decode, "avformat_find_stream_info", _inputUrl);
			UninitInput();
			return false;
		}

		for (unsigned int i = 0; i < _inputFormat->nb_streams; i++) {
			if (_inputFormat->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				_inputVideoIndex = i;
				break;
			}
		}
		if (_inputVideoIndex == -1) {
			LogPool::Error(LogEvent::Decode, "not found video index", _inputUrl);
			UninitInput();
			return false;
		}
		_inputStream = _inputFormat->streams[_inputVideoIndex];
		_sourceWidth = _inputStream->codecpar->width;
		_sourceHeight = _inputStream->codecpar->height;
		_channelStatus = ChannelStatus::Normal;
	}
	return true;
}

bool FFmpegChannel::InitOutput()
{
	if (_outputFormat == NULL && !_outputUrl.empty())
	{
		avformat_alloc_output_context2(&_outputFormat, NULL, "flv", _outputUrl.c_str());
		if (_outputFormat == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_alloc_output_context2", _outputUrl);
			UninitOutput();
			return false;
		}

		AVCodec* decode = avcodec_find_decoder(_inputStream->codecpar->codec_id);
		if (decode == NULL) {
			LogPool::Error(LogEvent::Decode, "avcodec_find_decoder", _inputUrl);
			UninitOutput();
			return false;
		}

		_outputStream = avformat_new_stream(_outputFormat, decode);
		if (_outputStream == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_new_stream", _outputUrl);
			UninitOutput();
			return false;
		}

		_outputCodec = avcodec_alloc_context3(decode);
		if (avcodec_parameters_to_context(_outputCodec, _inputStream->codecpar) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", _inputUrl);
			UninitOutput();
			return false;
		}
		_outputCodec->codec_tag = 0;
		if (_outputFormat->oformat->flags & AVFMT_GLOBALHEADER)
		{
			_outputCodec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

		if (avcodec_parameters_from_context(_outputStream->codecpar, _outputCodec) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", _outputUrl);
			UninitOutput();
			return false;
		}
		if (!(_outputFormat->oformat->flags & AVFMT_NOFILE))
		{
			if (avio_open(&_outputFormat->pb, _outputUrl.c_str(), AVIO_FLAG_WRITE))
			{
				LogPool::Error(LogEvent::Decode, "avio_open", _outputUrl);
				UninitOutput();
				return false;
			}
		}
		if (avformat_write_header(_outputFormat, NULL) < 0) {
			LogPool::Error(LogEvent::Decode, "avformat_write_header", _outputUrl);
			UninitOutput();
			return false;
		}
	}
	return true;
}

void FFmpegChannel::UninitInput()
{
	if (_inputFormat != NULL) {
		avformat_close_input(&_inputFormat);
		avformat_free_context(_inputFormat);
		_inputFormat = NULL;
	}
}

void FFmpegChannel::UninitOutput()
{
	if (_outputFormat != NULL)
	{
		if (_outputFormat->pb != NULL)
		{
			av_write_trailer(_outputFormat);
			avio_close(_outputFormat->pb);
		}	
		if (_outputCodec != NULL)
		{
			avcodec_close(_outputCodec);
		}
		avformat_free_context(_outputFormat);
		_outputFormat = NULL;
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

DecodeResult FFmpegChannel::Decode(const AVPacket* packet, int frameIndex, int frameSpan)
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
					_bgrHandler.HandleFrame(_bgrFrame->data[0], DestinationWidth, DestinationHeight, frameIndex);
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
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	int frameSpan = 0;
	long long duration = 0;
	int frameIndex = 1;
	while (!_cancelled)
	{
		if (_channelStatus == ChannelStatus::Normal)
		{
			if (frameSpan == 0)
			{
				frameSpan = 1000 / _inputStream->avg_frame_rate.num;
			}
			int readResult = av_read_frame(_inputFormat, &packet);
			LogPool::Debug(LogEvent::Decode, "frame", _inputUrl, frameIndex, readResult);
			if (readResult == AVERROR_EOF)
			{
				Decode(NULL, 0, frameSpan);
				if (_debug)
				{
					LogPool::Information(LogEvent::Decode, "read eof", _inputUrl);
					break;
				}
				else
				{
					duration += (frameIndex-1) * frameSpan;
					frameIndex = 1;
					_channelStatus = ChannelStatus::None;
				}
			}
			else if (readResult == 0)
			{
				if (packet.stream_index == _inputVideoIndex)
				{
					long long start = DateTime::UtcNowTimeStamp();
					DecodeResult decodeResult = Decode(&packet, frameIndex, frameSpan);
					if (decodeResult == DecodeResult::Error)
					{
						LogPool::Error(LogEvent::Decode, "decode error", _inputUrl, frameIndex);
						_channelStatus = ChannelStatus::DecodeError;
						break;
					}
					else if (decodeResult == DecodeResult::Handle)
					{
						//LogPool::Debug(LogEvent::Decode, "handle packet", _lastframeIndex, frameIndex);
						_frameSpan = frameIndex - _lastframeIndex;
						_lastframeIndex = frameIndex;
					}

					if (_outputFormat != NULL)
					{
						packet.pts = packet.pts == AV_NOPTS_VALUE ? duration + frameIndex * frameSpan : duration + av_rescale_q_rnd(packet.pts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
						packet.dts = duration + av_rescale_q_rnd(packet.dts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
						packet.duration = av_rescale_q(packet.duration, _inputStream->time_base, _outputStream->time_base);
						packet.pos = -1;
						av_write_frame(_outputFormat, &packet);
					}
					long long end = DateTime::UtcNowTimeStamp();
					long long sleepTime = frameSpan - (end - start);
					if (sleepTime > 0 && sleepTime <= frameSpan)
					{
						this_thread::sleep_for(chrono::milliseconds(sleepTime));
					}
					frameIndex += 1;
				}
				av_packet_unref(&packet);
			}
			else
			{
				LogPool::Error(LogEvent::Decode, "read error", _inputUrl,frameIndex, readResult);
				_channelStatus = ChannelStatus::ReadError;
				break;
			}
		}
		else
		{
			//重置不关闭输出
			UninitDecoder();
			UninitInput();
			if (InitInput())
			{
				if (InitOutput())
				{
					if (InitDecoder())
					{
						LogPool::Information(LogEvent::Decode, "init frame channel success", _inputUrl, _outputUrl);
						_channelStatus = ChannelStatus::Normal;
					}
					else
					{
						_channelStatus = ChannelStatus::DecoderError;
					}
				}
				else
				{
					_channelStatus = ChannelStatus::OutputError;
				}
			}
			else
			{
				_channelStatus = ChannelStatus::InputError;
			}
			if (_channelStatus != ChannelStatus::Normal)
			{
				this_thread::sleep_for(chrono::seconds(ConnectSpan));
			}
		}
	}
	UninitDecoder();
	UninitInput();
	UninitOutput();
}

