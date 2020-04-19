#include "FFmpegChannel.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;

FFmpegChannel::FFmpegChannel()
	:FrameChannel(),_decodeContext(NULL),  _yuvFrame(NULL),_bgrFrame(NULL), _bgrBuffer(NULL), _bgrSwsContext(NULL)
{
	_h264Handler = new H264Handler();
	_yuvHandler = new YUV420PHandler();
	_bgrHandler = new BGR24Handler();
}

int FFmpegChannel::InitCore()
{
	if (_formatContext->streams[_videoIndex]->codecpar->codec_id != AVCodecID::AV_CODEC_ID_H264)
	{
		LogPool::Warning(LogEvent::Decode, "codec is not h264", _url);
		return 4;
	}

	AVCodec* decode = avcodec_find_decoder(_formatContext->streams[_videoIndex]->codecpar->codec_id);
	if (decode == NULL) {
		LogPool::Warning(LogEvent::Decode, "avcodec_find_decoder error", _url);
		return 5;
	}
	_decodeContext = avcodec_alloc_context3(decode);
	if (avcodec_parameters_to_context(_decodeContext, _formatContext->streams[_videoIndex]->codecpar) < 0) {
		LogPool::Warning(LogEvent::Decode, "avcodec_parameters_to_context error", _url);
		return 6;
	}

	if (avcodec_open2(_decodeContext, decode, NULL) < 0) {//打开解码器
		LogPool::Warning(LogEvent::Decode, "avcodec_open2 error", _url);
		return 7;
	}

	//初始化帧
	_yuvFrame = av_frame_alloc();

	//yuv转rgb
	_bgrFrame = av_frame_alloc();
	_bgrBuffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_BGR24, _decodeContext->width, _decodeContext->height, 1));
	_bgrWidth = _decodeContext->width;
	_bgrHeight = _decodeContext->height;
	if (av_image_fill_arrays(_bgrFrame->data, _bgrFrame->linesize, _bgrBuffer, AV_PIX_FMT_BGR24, _decodeContext->width, _decodeContext->height, 1) < 0)
	{
		LogPool::Warning(LogEvent::Decode, "av_image_fill_arrays error");
		return 8;
	}
	_bgrSwsContext = sws_getContext(_decodeContext->width, _decodeContext->height, _decodeContext->pix_fmt, _decodeContext->width, _decodeContext->height, AV_PIX_FMT_BGR24,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (_bgrSwsContext == NULL)
	{
		LogPool::Warning(LogEvent::Decode, "sws_getContext error", _url);
		return 9;
	}
	return 0;
}

void FFmpegChannel::UninitCore()
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

bool FFmpegChannel::Decode(const AVPacket* packet, int packetIndex)
{
	if (avcodec_send_packet(_decodeContext, packet) == 0)
	{
		_h264Handler->HandleFrame(packet->data, packet->size);
		while (true)
		{
			int resultReceive = avcodec_receive_frame(_decodeContext, _yuvFrame);
			if (resultReceive == AVERROR(EAGAIN) || resultReceive == AVERROR_EOF) 
			{
				break;
			}
			else if (resultReceive < 0)
			{
				LogPool::Warning(LogEvent::Decode, "receive error", _url);
				return false;
			}
			else if (resultReceive >= 0)
			{
				_yuvHandler->HandleFrame(_yuvFrame->data[0], _decodeContext->width, _decodeContext->height, packetIndex);
				if (sws_scale(_bgrSwsContext, _yuvFrame->data,
					_yuvFrame->linesize, 0, _decodeContext->height,
					_bgrFrame->data, _bgrFrame->linesize) != 0)
				{
					_bgrHandler->HandleFrame(_bgrFrame->data[0], _decodeContext->width, _decodeContext->height, packetIndex);
				}
			}
		}
	}
	else
	{
		LogPool::Warning(LogEvent::Decode, "send error", _url);
		return false;
	}
	return true;
}
