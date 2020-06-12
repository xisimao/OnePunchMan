#include "FFmpegChannel.h"

using namespace std;
using namespace OnePunchMan;

const int FFmpegChannel::ConnectSpan = 5000;
const int FFmpegChannel::DestinationWidth = 1920;
const int FFmpegChannel::DestinationHeight = 1080;

FFmpegChannel::FFmpegChannel(bool debug)
	:ThreadObject("decode"), _debug(debug)
	, _inputUrl(), _inputFormat(NULL), _inputStream(NULL), _inputVideoIndex(-1)
	, _outputUrl(), _outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL)
	, _channelStatus(ChannelStatus::Init), _options(NULL), _sourceWidth(0), _sourceHeight(0)
	, _decodeContext(NULL), _yuvFrame(NULL), _bgrFrame(NULL), _bgrBuffer(NULL), _bgrSwsContext(NULL)
	, _lastframeIndex(0), _frameSpan(0)
{

}

void FFmpegChannel::InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
	LogPool::Information(LogEvent::Decode, "init ffmpeg sdk");
}

void FFmpegChannel::UninitFFmpeg()
{
	avformat_network_deinit();
	LogPool::Information(LogEvent::Decode, "uninit video sdk");
}

string FFmpegChannel::InputUrl()
{
	lock_guard<mutex> lck(_mutex);
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

void FFmpegChannel::UpdateChannel(const std::string& inputUrl, const std::string& outputUrl)
{
	lock_guard<mutex> lck(_mutex);
	_inputUrl.assign(inputUrl);
	_outputUrl.assign(outputUrl);
}

void FFmpegChannel::ClearChannel()
{
	lock_guard<mutex> lck(_mutex);
	_inputUrl.clear();
	_outputUrl.clear();
}

ChannelStatus FFmpegChannel::InitInput(const string& inputUrl)
{
	if (inputUrl.empty())
	{
		return ChannelStatus::Init;
	}
	else
	{
		if (_inputFormat == NULL)
		{
			if (avformat_open_input(&_inputFormat, inputUrl.c_str(), NULL, &_options) != 0) {
				LogPool::Error(LogEvent::Decode, "avformat_open_input", inputUrl);
				UninitInput();
				return ChannelStatus::InputError;
			}

			if (avformat_find_stream_info(_inputFormat, NULL) < 0) {
				LogPool::Error(LogEvent::Decode, "avformat_find_stream_info", inputUrl);
				UninitInput();
				return ChannelStatus::InputError;
			}

			for (unsigned int i = 0; i < _inputFormat->nb_streams; i++) {
				if (_inputFormat->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
					_inputVideoIndex = i;
					break;
				}
			}
			if (_inputVideoIndex == -1) {
				LogPool::Error(LogEvent::Decode, "not found video index", inputUrl);
				UninitInput();
				return ChannelStatus::InputError;
			}
			_inputStream = _inputFormat->streams[_inputVideoIndex];
			_sourceWidth = _inputStream->codecpar->width;
			_sourceHeight = _inputStream->codecpar->height;
		}
		return ChannelStatus::Normal;
	}
}

ChannelStatus FFmpegChannel::InitOutput(const string& outputUrl)
{
	if (_outputFormat == NULL && !outputUrl.empty())
	{
		avformat_alloc_output_context2(&_outputFormat, NULL, "flv", outputUrl.c_str());
		if (_outputFormat == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_alloc_output_context2", outputUrl);
			UninitOutput();
			return ChannelStatus::OutputError;
		}

		AVCodec* decode = avcodec_find_decoder(_inputStream->codecpar->codec_id);
		if (decode == NULL) {
			LogPool::Error(LogEvent::Decode, "avcodec_find_decoder", outputUrl);
			UninitOutput();
			return ChannelStatus::OutputError;
		}

		_outputStream = avformat_new_stream(_outputFormat, decode);
		if (_outputStream == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_new_stream", outputUrl);
			UninitOutput();
			return ChannelStatus::OutputError;
		}

		_outputCodec = avcodec_alloc_context3(decode);
		if (avcodec_parameters_to_context(_outputCodec, _inputStream->codecpar) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", outputUrl);
			UninitOutput();
			return ChannelStatus::OutputError;
		}
		_outputCodec->codec_tag = 0;
		if (_outputFormat->oformat->flags & AVFMT_GLOBALHEADER)
		{
			_outputCodec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

		if (avcodec_parameters_from_context(_outputStream->codecpar, _outputCodec) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", outputUrl);
			UninitOutput();
			return ChannelStatus::OutputError;
		}
		if (!(_outputFormat->oformat->flags & AVFMT_NOFILE))
		{
			if (avio_open(&_outputFormat->pb, outputUrl.c_str(), AVIO_FLAG_WRITE))
			{
				LogPool::Error(LogEvent::Decode, "avio_open", outputUrl);
				UninitOutput();
				return ChannelStatus::OutputError;
			}
		}
		if (avformat_write_header(_outputFormat, NULL) < 0) {
			LogPool::Error(LogEvent::Decode, "avformat_write_header", outputUrl);
			UninitOutput();
			return ChannelStatus::OutputError;
		}
	}
	return ChannelStatus::Normal;
}

void FFmpegChannel::UninitInput()
{
	if (_inputFormat != NULL) {
		_inputVideoIndex = -1;
		_inputStream = NULL;
		avformat_close_input(&_inputFormat);
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
			avio_closep(&_outputFormat->pb);
		}	
		if (_outputCodec != NULL)
		{
			avcodec_free_context(&_outputCodec);
		}
		avformat_free_context(_outputFormat);
		_outputCodec = NULL;
		_outputStream = NULL;
		_outputFormat = NULL;
	}
}

ChannelStatus FFmpegChannel::InitDecoder(const string& inputUrl)
{
	if (_inputStream->codecpar->codec_id != AVCodecID::AV_CODEC_ID_H264)
	{
		LogPool::Warning(LogEvent::Decode, "codec is not h264", inputUrl);
		return ChannelStatus::DecoderError;
	}

	AVCodec* decode = avcodec_find_decoder(_inputStream->codecpar->codec_id);
	if (decode == NULL) {
		LogPool::Warning(LogEvent::Decode, "avcodec_find_decoder error", inputUrl);
		return ChannelStatus::DecoderError;
	}
	_decodeContext = avcodec_alloc_context3(decode);
	if (avcodec_parameters_to_context(_decodeContext, _inputStream->codecpar) < 0) {
		LogPool::Warning(LogEvent::Decode, "avcodec_parameters_to_context error", inputUrl);
		return ChannelStatus::DecoderError;
	}

	if (avcodec_open2(_decodeContext, decode, NULL) < 0) {//打开解码器
		LogPool::Warning(LogEvent::Decode, "avcodec_open2 error", inputUrl);
		return ChannelStatus::DecoderError;
	}

	//初始化帧
	_yuvFrame = av_frame_alloc();

	//yuv转rgb
	_bgrFrame = av_frame_alloc();
	_bgrBuffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_BGR24, DestinationWidth, DestinationHeight, 1));
	if (av_image_fill_arrays(_bgrFrame->data, _bgrFrame->linesize, _bgrBuffer, AV_PIX_FMT_BGR24, DestinationWidth, DestinationHeight, 1) < 0)
	{
		LogPool::Warning(LogEvent::Decode, "av_image_fill_arrays error");
		return ChannelStatus::DecoderError;
	}
	_bgrSwsContext = sws_getContext(_decodeContext->width, _decodeContext->height, _decodeContext->pix_fmt, DestinationWidth, DestinationHeight, AV_PIX_FMT_BGR24,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (_bgrSwsContext == NULL)
	{
		LogPool::Warning(LogEvent::Decode, "sws_getContext error", inputUrl);
		return ChannelStatus::DecoderError;
	}
	return ChannelStatus::Normal;
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
		return DecodeResult::Error;
	}
	return DecodeResult::Handle;
}

void FFmpegChannel::StartCore()
{
	AVPacket* packet= av_packet_alloc();
	int frameTimeSpan = 0;
	long long duration = 0;
	int frameIndex = 1;
	string inputUrl;
	string outputUrl;
	while (!_cancelled)
	{
		unique_lock<mutex> lck(_mutex);
		if (inputUrl.compare(_inputUrl) != 0)
		{
			_channelStatus = ChannelStatus::Init;
			inputUrl = _inputUrl;
			outputUrl = _outputUrl;
		}
		lck.unlock();
		if (_channelStatus == ChannelStatus::Normal)
		{
			if (frameTimeSpan == 0)
			{
				frameTimeSpan = 1000 /(_inputStream->avg_frame_rate.num/ _inputStream->avg_frame_rate.den);
			}
			long long timeStamp1 = DateTime::UtcNowTimeStamp();
			int readResult = av_read_frame(_inputFormat, packet);
			if (readResult == AVERROR_EOF)
			{
				Decode(NULL, 0, frameTimeSpan);
				if (_debug)
				{
					LogPool::Information(LogEvent::Decode, "read eof", inputUrl);
					av_packet_unref(packet);
					break;
				}
				else
				{
					duration += (frameIndex-1) * frameTimeSpan;
					frameIndex = 1;
					_channelStatus = ChannelStatus::ReadEOF;
				}
			}
			else if (readResult == 0)
			{
				if (packet->stream_index == _inputVideoIndex)
				{
					long long timeStamp2 = DateTime::UtcNowTimeStamp();
					DecodeResult decodeResult = Decode(packet, frameIndex, frameTimeSpan);
					long long timeStamp3 = DateTime::UtcNowTimeStamp();
					if (decodeResult == DecodeResult::Error)
					{
						LogPool::Error(LogEvent::Decode, "decode error", inputUrl, frameIndex);
						_channelStatus = ChannelStatus::DecodeError;
						av_packet_unref(packet);
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
						packet->pts = packet->pts == AV_NOPTS_VALUE ? duration + frameIndex * frameTimeSpan : duration + av_rescale_q_rnd(packet->pts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
						packet->dts = duration + av_rescale_q_rnd(packet->dts, _inputStream->time_base, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
						packet->duration = av_rescale_q(packet->duration, _inputStream->time_base, _outputStream->time_base);
						packet->pos = -1;
						av_interleaved_write_frame(_outputFormat, packet);
					}
					long long timeStamp4 = DateTime::UtcNowTimeStamp();
					long long sleepTime = frameTimeSpan - (timeStamp4 - timeStamp2);
					if (sleepTime > 0 && sleepTime <= frameTimeSpan)
					{
						this_thread::sleep_for(chrono::milliseconds(sleepTime));
					}
					LogPool::Debug(LogEvent::Decode, "frame", inputUrl, frameIndex, static_cast<int>(decodeResult),timeStamp2-timeStamp1,timeStamp3-timeStamp2,timeStamp4-timeStamp3);
					frameIndex += 1;
				}
			}
			else
			{
				LogPool::Error(LogEvent::Decode, "read error", inputUrl, frameIndex, readResult);
				_channelStatus = ChannelStatus::ReadError;
			}
			av_packet_unref(packet);
		}
		else
		{
			UninitDecoder();
			//文件循环时不关闭输出
			if (_channelStatus != ChannelStatus::ReadEOF)
			{
				UninitOutput();
			}
			UninitInput();
			ChannelStatus inputStatus = InitInput(inputUrl);
			if(inputStatus ==ChannelStatus::Normal)
			{
				ChannelStatus outputStatus = InitOutput(outputUrl);
				if (outputStatus == ChannelStatus::Normal)
				{
					ChannelStatus decoderStatus = InitDecoder(inputUrl);
					if (decoderStatus == ChannelStatus::Normal)
					{
						LogPool::Information(LogEvent::Decode, "init frame channel success", inputUrl, outputUrl);
						_channelStatus = ChannelStatus::Normal;
					}
					else
					{
						_channelStatus = decoderStatus;
					}
				}
				else
				{
					_channelStatus = outputStatus;
				}
			}
			else
			{
				_channelStatus = inputStatus;
			}
			if (_channelStatus != ChannelStatus::Normal)
			{
				this_thread::sleep_for(chrono::milliseconds(ConnectSpan));
			}
		}
	}
	av_packet_free(&packet);
	UninitDecoder();
	UninitInput();
	UninitOutput();
}

