#include "OutputHandler.h"

using namespace std;
using namespace OnePunchMan;

OutputHandler::OutputHandler()
	:_outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL), _inputTimeBase(), _channelIndex(0),_frameIndex(1),_frameCount(0), _frameSpan(0), _ptsBase(0)
{
}

ChannelStatus OutputHandler::Init(const string& outputUrl, const InputHandler& inputHandler)
{
	if (_outputFormat == NULL && !outputUrl.empty())
	{
		if (outputUrl.size() >= 4 && outputUrl.substr(0, 4).compare("rtmp") == 0)
		{
			avformat_alloc_output_context2(&_outputFormat, NULL, "flv", outputUrl.c_str());
		}
		else
		{
			avformat_alloc_output_context2(&_outputFormat, NULL, NULL, outputUrl.c_str());
		}
		if (_outputFormat == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_alloc_output_context2", outputUrl);
			Uninit();
			return ChannelStatus::OutputError;
		}

		AVCodec* decode = avcodec_find_decoder(inputHandler.Stream()->codecpar->codec_id);
		if (decode == NULL) {
			LogPool::Error(LogEvent::Decode, "avcodec_find_decoder", outputUrl);
			Uninit();
			return ChannelStatus::OutputError;
		}

		_outputStream = avformat_new_stream(_outputFormat, decode);
		if (_outputStream == NULL) {
			LogPool::Error(LogEvent::Decode, "avformat_new_stream", outputUrl);
			Uninit();
			return ChannelStatus::OutputError;
		}

		_outputCodec = avcodec_alloc_context3(decode);
		if (avcodec_parameters_to_context(_outputCodec, inputHandler.Stream()->codecpar) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", outputUrl);
			Uninit();
			return ChannelStatus::OutputError;
		}
		_outputCodec->codec_tag = 0;
		if (_outputFormat->oformat->flags & AVFMT_GLOBALHEADER)
		{
			_outputCodec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

		if (avcodec_parameters_from_context(_outputStream->codecpar, _outputCodec) < 0) {
			LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context", outputUrl);
			Uninit();
			return ChannelStatus::OutputError;
		}
		if (!(_outputFormat->oformat->flags & AVFMT_NOFILE))
		{
			if (avio_open(&_outputFormat->pb, outputUrl.c_str(), AVIO_FLAG_WRITE))
			{
				LogPool::Error(LogEvent::Decode, "avio_open", outputUrl);
				Uninit();
				return ChannelStatus::OutputError;
			}
		}
		if (avformat_write_header(_outputFormat, NULL) < 0) {
			LogPool::Error(LogEvent::Decode, "avformat_write_header", outputUrl);
			Uninit();
			return ChannelStatus::OutputError;
		}
	}
	_inputTimeBase = inputHandler.Stream()->time_base;
	_frameIndex = 1;
	_frameSpan = inputHandler.FrameSpan();
	_ptsBase = (inputHandler.Stream()->time_base.den) / inputHandler.Stream()->time_base.num / (1000 / inputHandler.FrameSpan());
	return ChannelStatus::Normal;
}

ChannelStatus OutputHandler::Init(int channelIndex, const string& outputUrl, const string& inputUrl,int frameCount)
{
	InputHandler handler;
	ChannelStatus status=handler.Init(inputUrl);
	if (status == ChannelStatus::Normal)
	{
		status =Init(outputUrl, handler);
	}
	handler.Uninit();
	_channelIndex = channelIndex;
	_frameCount = frameCount;
	return status;
}

void OutputHandler::Uninit()
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

void OutputHandler::PushRtmpPacket(AVPacket* packet, unsigned int frameIndex, long long duration)
{
	if (_outputFormat != NULL)
	{
		packet->pts = packet->pts == AV_NOPTS_VALUE ? duration + frameIndex * _frameSpan : duration + av_rescale_q_rnd(packet->pts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		if (packet->dts!= AV_NOPTS_VALUE)
		{
			packet->dts = duration + av_rescale_q_rnd(packet->dts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		}
		packet->duration = av_rescale_q(packet->duration, _inputTimeBase, _outputStream->time_base);
		packet->pos = -1;
		av_interleaved_write_frame(_outputFormat, packet);
	}
}

bool OutputHandler::PushMp4Packet(unsigned char* data, int size)
{
	if (_frameIndex <= _frameCount&&_outputFormat!=NULL)
	{
		AVPacket* packet = av_packet_alloc();
		unsigned char* temp = (unsigned char*)av_malloc(size);
		memcpy(temp, data, size);
		av_packet_from_data(packet, temp, size);
		packet->pts = _frameIndex * _ptsBase;
		packet->duration = av_rescale_q(packet->duration, _inputTimeBase, _outputStream->time_base);
		packet->pos = -1;
		av_interleaved_write_frame(_outputFormat, packet);
		av_packet_free(&packet);
		_frameIndex += 1;
		return false;
	}
	else
	{
		return true;
	}
}