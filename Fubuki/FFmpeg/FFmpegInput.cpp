#include "FFmpegInput.h"

using namespace std;
using namespace OnePunchMan;

FFmpegInput::FFmpegInput()
	:_inputUrl(), _inputFormat(NULL), _inputStream(NULL), _inputVideoIndex(-1),_sourceWidth(0),_sourceHeight(0),_frameSpan(0)
{

}

void FFmpegInput::InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
	LogPool::Information(LogEvent::Decode, "init ffmpeg sdk");
}

void FFmpegInput::UninitFFmpeg()
{
	avformat_network_deinit();
	LogPool::Information(LogEvent::Decode, "uninit video sdk");
}

AVFormatContext* FFmpegInput::FormatContext()
{
	return _inputFormat;
}

AVStream* FFmpegInput::Stream() const
{
	return _inputStream;
}

int FFmpegInput::VideoIndex() const
{
	return _inputVideoIndex;
}

int FFmpegInput::SourceWidth() const
{
	return _sourceWidth;
}

int FFmpegInput::SourceHeight() const
{
	return _sourceHeight;
}

unsigned char FFmpegInput::FrameSpan() const
{
	return _frameSpan;
}

ChannelStatus FFmpegInput::Init(const string& inputUrl)
{
	if (inputUrl.empty())
	{
		return ChannelStatus::Init;
	}
	else
	{
		if (_inputFormat == NULL)
		{
			AVDictionary* options=NULL;
			if (inputUrl.size() >= 4 && inputUrl.substr(0, 4).compare("rtsp") == 0)
			{
				av_dict_set(&options, "rtsp_transport", "tcp", 0);
				av_dict_set(&options, "stimeout", "5000000", 0);
			}
			if (avformat_open_input(&_inputFormat, inputUrl.c_str(), 0,0) != 0) {
				LogPool::Error(LogEvent::Decode, "avformat_open_input", inputUrl);
				Uninit();
				return ChannelStatus::InputError;
			}

			if (avformat_find_stream_info(_inputFormat, NULL) < 0) {
				LogPool::Error(LogEvent::Decode, "avformat_find_stream_info", inputUrl);
				Uninit();
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
				Uninit();
				return ChannelStatus::InputError;
			}
			_inputStream = _inputFormat->streams[_inputVideoIndex];
			_sourceWidth = _inputStream->codecpar->width;
			_sourceHeight = _inputStream->codecpar->height;
			if (_inputStream->avg_frame_rate.den != 0)
			{
				_frameSpan = static_cast<unsigned char>(1000 / (_inputStream->avg_frame_rate.num / _inputStream->avg_frame_rate.den));
			}
			else if (_inputStream->r_frame_rate.den != 0)
			{
				_frameSpan = static_cast<unsigned char>(1000 / (_inputStream->r_frame_rate.num / _inputStream->r_frame_rate.den));
			}
			else
			{
				_frameSpan = 40;
				LogPool::Warning(LogEvent::Decode, "frame span not found", _inputUrl);
			}
		}
		return ChannelStatus::Normal;
	}
}

void FFmpegInput::Uninit()
{
	if (_inputFormat != NULL) {
		_inputVideoIndex = -1;
		_inputStream = NULL;
		avformat_close_input(&_inputFormat);
		_inputFormat = NULL;
		_sourceWidth = 0;
		_sourceHeight = 0;
		_frameSpan = 0;
	}
}
