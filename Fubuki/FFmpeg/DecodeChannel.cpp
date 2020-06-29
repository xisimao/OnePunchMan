#include "DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;

const int DecodeChannel::ConnectSpan = 5000;
const int DecodeChannel::DestinationWidth = 1920;
const int DecodeChannel::DestinationHeight = 1080;

DecodeChannel::DecodeChannel(int channelIndex)
	:ThreadObject("decode"), _channelIndex(channelIndex)
	, _inputUrl(), _inputHandler()
	, _outputUrl(), _outputHandler()
	, _taskId(0), _channelStatus(ChannelStatus::Init), _loop(false), _frameSpan(0)
	, _decodeContext(NULL), _yuvFrame(NULL), _bgrFrame(NULL), _bgrBuffer(NULL), _bgrSwsContext(NULL)
	, _lastframeIndex(0), _handleSpan(0)
{

}

void DecodeChannel::InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
	LogPool::Information(LogEvent::Decode, "init ffmpeg sdk");
}

void DecodeChannel::UninitFFmpeg()
{
	avformat_network_deinit();
	LogPool::Information(LogEvent::Decode, "uninit video sdk");
}

string DecodeChannel::InputUrl()
{
	lock_guard<mutex> lck(_mutex);
	return _inputUrl;
}

ChannelStatus DecodeChannel::Status()
{
	return _channelStatus;
}

int DecodeChannel::SourceWidth()
{
	return _inputHandler.SourceWidth();
}

int DecodeChannel::SourceHeight()
{
	return _inputHandler.SourceHeight();
}

int DecodeChannel::HandleSpan()
{
	return _handleSpan;
}

int DecodeChannel::FrameSpan()
{
	return static_cast<int>(_frameSpan);
}

unsigned char DecodeChannel::UpdateChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop)
{
	lock_guard<mutex> lck(_mutex);
	if (_inputUrl.compare(inputUrl) != 0
		|| !loop
		|| _channelStatus != ChannelStatus::Normal)
	{
		_channelStatus = ChannelStatus::Init;
	}
	_inputUrl.assign(inputUrl);
	_outputUrl.assign(outputUrl);
#ifdef _WIN32
	_outputUrl.clear();
#endif // _WIN32
	_loop = loop;
	return ++_taskId;
}

void DecodeChannel::ClearChannel()
{
	lock_guard<mutex> lck(_mutex);
	_inputUrl.clear();
	_outputUrl.clear();
	_channelStatus = ChannelStatus::Init;
}

ChannelStatus DecodeChannel::InitDecoder(const string& inputUrl)
{
	if (_inputHandler.Stream()->codecpar->codec_id != AVCodecID::AV_CODEC_ID_H264)
	{
		LogPool::Warning(LogEvent::Decode, "codec is not h264", inputUrl);
		return ChannelStatus::DecoderError;
	}

	AVCodec* decode = avcodec_find_decoder(_inputHandler.Stream()->codecpar->codec_id);
	if (decode == NULL) {
		LogPool::Warning(LogEvent::Decode, "avcodec_find_decoder error", inputUrl);
		return ChannelStatus::DecoderError;
	}
	_decodeContext = avcodec_alloc_context3(decode);
	if (avcodec_parameters_to_context(_decodeContext, _inputHandler.Stream()->codecpar) < 0) {
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

void DecodeChannel::UninitDecoder()
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

DecodeResult DecodeChannel::Decode(const AVPacket* packet, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan)
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

void DecodeChannel::StartCore()
{
	AVPacket* packet = av_packet_alloc();
	long long duration = 0;
	unsigned char taskId = 0;
	unsigned int frameIndex = 1;
	bool reportFinish = false;
	string inputUrl;
	while (!_cancelled)
	{
		if (_channelStatus == ChannelStatus::Normal)
		{
			long long timeStamp1 = DateTime::UtcNowTimeStamp();
			int readResult = av_read_frame(_inputHandler.FormatContext(), packet);
			if (readResult == AVERROR_EOF)
			{
				if (_loop)
				{
					_channelStatus = ChannelStatus::ReadEOF_Restart;
				}
				else
				{
					LogPool::Information(LogEvent::Decode, "read eof", _channelIndex);
					_channelStatus = ChannelStatus::ReadEOF_Stop;
				}
			}
			else if (readResult == 0)
			{
				if (packet->stream_index == _inputHandler.VideoIndex())
				{
					long long timeStamp2 = DateTime::UtcNowTimeStamp();
					DecodeResult decodeResult = Decode(packet, taskId, frameIndex, _frameSpan);
					long long timeStamp3 = DateTime::UtcNowTimeStamp();
					if (decodeResult == DecodeResult::Error)
					{
						LogPool::Error(LogEvent::Decode, "decode error", _channelIndex, frameIndex);
						_channelStatus = ChannelStatus::DecodeError;
					}
					else if (decodeResult == DecodeResult::Handle)
					{
						_handleSpan = frameIndex - _lastframeIndex;
						_lastframeIndex = frameIndex;
					}
					_outputHandler.PushPacket(packet, frameIndex, duration);

					long long timeStamp4 = DateTime::UtcNowTimeStamp();
					long long sleepTime = _frameSpan - (timeStamp4 - timeStamp2);
					if (sleepTime > 0 && sleepTime <= _frameSpan)
					{
						this_thread::sleep_for(chrono::milliseconds(sleepTime));
					}
					LogPool::Debug(LogEvent::Decode, "frame", _channelIndex, static_cast<int>(taskId), frameIndex, static_cast<int>(_frameSpan), static_cast<int>(decodeResult), timeStamp2 - timeStamp1, timeStamp3 - timeStamp2, timeStamp4 - timeStamp3);
					frameIndex += 1;
				}
			}
			else
			{
				LogPool::Error(LogEvent::Decode, "read error", _channelIndex, frameIndex, readResult);
				_channelStatus = ChannelStatus::ReadError;
			}
			av_packet_unref(packet);
		}
		else
		{
			//播放文件结束时报一次结束
			if (_channelStatus == ChannelStatus::ReadEOF_Stop && !reportFinish)
			{
				Decode(NULL, taskId, frameIndex, _frameSpan);
				reportFinish = true;
			}
			unique_lock<mutex> lck(_mutex);
			UninitDecoder();
			//读取到结尾重新启动时不需要重置输出
			if (_channelStatus != ChannelStatus::ReadEOF_Restart)
			{
				//UninitOutput();
				_outputHandler.Uninit();
			}
			_inputHandler.Uninit();
			ChannelStatus oldStatus = _channelStatus;

			//循环停止和解码错误时不再重新打开视频
			if (_channelStatus != ChannelStatus::ReadEOF_Stop
				&& _channelStatus != ChannelStatus::DecodeError)
			{
				ChannelStatus inputStatus = _inputHandler.Init(_inputUrl);
				if (inputStatus == ChannelStatus::Normal)
				{
					ChannelStatus outputStatus = _outputHandler.Init(_outputUrl, _inputHandler);
					if (outputStatus == ChannelStatus::Normal)
					{
						ChannelStatus decoderStatus = InitDecoder(_inputUrl);
						if (decoderStatus == ChannelStatus::Normal)
						{
							_channelStatus = ChannelStatus::Normal;
						}
						else
						{
							_channelStatus = decoderStatus;
						}
					}
					else
					{
						_channelStatus = ChannelStatus::OutputError;
					}
				}
				else
				{
					_channelStatus = inputStatus;
				}
			}

			if (_channelStatus == ChannelStatus::Normal)
			{
				//只有在视频循环时才不重置参数,而对时长进行累加
				if (inputUrl.compare(_inputUrl) == 0
					&& oldStatus == ChannelStatus::ReadEOF_Restart)
				{
					duration += (frameIndex - 1) * _frameSpan;
				}
				else
				{
					duration = 0;
					_frameSpan = _inputHandler.FrameSpan();
				}
				taskId = _taskId;
				frameIndex = 1;
				reportFinish = false;
				inputUrl = _inputUrl;
				LogPool::Information(LogEvent::Decode, "init frame channel success,inputUrl:", _inputUrl, "outputUrl:", _outputUrl, "frame span:", static_cast<int>(_frameSpan), "loop:", _loop);
				lck.unlock();
			}
			else
			{
				lck.unlock();
				this_thread::sleep_for(chrono::milliseconds(ConnectSpan));
			}
		}
	}
	av_packet_free(&packet);
	UninitDecoder();
	_inputHandler.Uninit();
	_outputHandler.Uninit();
}

