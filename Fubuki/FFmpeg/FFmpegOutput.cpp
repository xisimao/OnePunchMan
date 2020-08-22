#include "FFmpegOutput.h"

using namespace std;
using namespace OnePunchMan;

FFmpegOutput::FFmpegOutput(const std::string& outputUrl, int iFrameCount)
	: _outputUrl(outputUrl),_outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL)
	, _iFrameIndex(0),_iFrameCount(iFrameCount), _frameIndex(0), _frameSpan(0)
{

}

bool FFmpegOutput::Init(const AVCodecParameters* parameters)
{
	if (_outputFormat != NULL)
	{
		LogPool::Warning(LogEvent::Encode, "已经初始化了输出文件", _outputUrl);
		return false;
	}
	if (_outputUrl.size() >= 4 && _outputUrl.substr(0, 4).compare("rtmp") == 0)
	{
		avformat_alloc_output_context2(&_outputFormat, NULL, "flv", _outputUrl.c_str());
		_frameSpan = 40;
	}
	else
	{
		avformat_alloc_output_context2(&_outputFormat, NULL, NULL, _outputUrl.c_str());
		_frameSpan = 3600;
	}

	if (_outputFormat == NULL) {
		LogPool::Error(LogEvent::Encode, "avformat_alloc_output_context2", _outputUrl);
		Uninit();
		return false;
	}
	AVCodec* decode = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (decode == NULL) {
		LogPool::Error(LogEvent::Encode, "avcodec_find_decoder", _outputUrl);
		Uninit();
		return false;
	}
	_outputStream = avformat_new_stream(_outputFormat, decode);
	if (_outputStream == NULL) {
		LogPool::Error(LogEvent::Encode, "avformat_new_stream", _outputUrl);
		Uninit();
		return false;
	}
	_outputCodec = avcodec_alloc_context3(decode);
	if (avcodec_parameters_to_context(_outputCodec, parameters) < 0) {
		LogPool::Error(LogEvent::Encode, "avcodec_parameters_to_context", _outputUrl);
		Uninit();
		return false;
	}
	_outputCodec->codec_tag = 0;
	if (_outputFormat->oformat->flags & AVFMT_GLOBALHEADER)
	{
		_outputCodec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	if (avcodec_parameters_from_context(_outputStream->codecpar, _outputCodec) < 0) {
		LogPool::Error(LogEvent::Encode, "avcodec_parameters_to_context", _outputUrl);
		Uninit();
		return false;
	}
	if (!(_outputFormat->oformat->flags & AVFMT_NOFILE))
	{
		if (avio_open(&_outputFormat->pb, _outputUrl.c_str(), AVIO_FLAG_WRITE))
		{
			LogPool::Error(LogEvent::Encode, "avio_open", _outputUrl);
			Uninit();
			return false;
		}
	}
	if (avformat_write_header(_outputFormat, NULL) < 0) {
		LogPool::Error(LogEvent::Encode, "avformat_write_header", _outputUrl);
		Uninit();
		return false;
	}
	LogPool::Information(LogEvent::Encode, "初始化输出文件:", _outputUrl);
	return true;
}

void FFmpegOutput::Uninit()
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
	LogPool::Information("结束输出文件:",_outputUrl,"共输出:", _frameIndex,"帧");
}

bool FFmpegOutput::Finished()
{
	return _outputFormat == NULL;
}

void FFmpegOutput::WritePacket(const unsigned char* data,int size,int frameType)
{
	if (_outputFormat == NULL)
	{
		return;
	}
	else
	{
		if (frameType == 5)
		{
			_iFrameIndex += 1;
			if (_iFrameCount!=-1&&_iFrameIndex > _iFrameCount)
			{
				Uninit();
				return;
			}
		}
		//packet->pts = packet->pts == AV_NOPTS_VALUE ? duration + frameIndex * _frameSpan : duration + av_rescale_q_rnd(packet->pts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//if (packet->dts!= AV_NOPTS_VALUE)
		//{
		//	packet->dts = duration + av_rescale_q_rnd(packet->dts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//}
		//packet->duration = av_rescale_q(packet->duration, _inputTimeBase, _outputStream->time_base);

		AVPacket* packet = av_packet_alloc();
		unsigned char* temp = (unsigned char*)av_malloc(size);
		memcpy(temp, data, size);
		av_packet_from_data(packet, temp, size);
		packet->flags = frameType == 5 ? 1 : 0;
		packet->pos = -1;
		packet->pts = _frameIndex * _frameSpan;
		packet->dts = packet->pts;
		packet->duration = _frameSpan;
		av_interleaved_write_frame(_outputFormat, packet);
		av_packet_free(&packet);
		_frameIndex += 1;
	}
}
