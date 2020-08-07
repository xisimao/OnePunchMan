#include "FFmpegOutput.h"

using namespace std;
using namespace OnePunchMan;

FFmpegOutput::FFmpegOutput()
	:_outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL), _inputTimeBase(), _channelIndex(0),_frameIndex(1),_frameCount(0), _frameSpan(0), _ptsBase(0)
{
}

ChannelStatus FFmpegOutput::Init(const string& outputUrl, const FFmpegInput& inputHandler)
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

		AVCodec* decode = avcodec_find_decoder(AV_CODEC_ID_H264);
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

		AVCodecParameters b;
		b.codec_type = AVMEDIA_TYPE_VIDEO;
		b.codec_id = AV_CODEC_ID_H264;
		b.codec_tag = 0;
		b.format = 0;
		b.bit_rate = 0;
		b.bits_per_coded_sample = 0;
		b.bits_per_raw_sample = 8;
		b.profile = 66;
		b.level = 51;
		b.width = 2752;
		b.height = 2208;
		b.sample_aspect_ratio.num = 0;
		b.sample_aspect_ratio.den = 1;
		b.field_order = AV_FIELD_PROGRESSIVE;
		b.color_range = AVCOL_RANGE_UNSPECIFIED;
		b.color_primaries = AVCOL_PRI_UNSPECIFIED;
		b.color_trc = AVCOL_TRC_UNSPECIFIED;
		b.color_space = AVCOL_SPC_UNSPECIFIED;
		b.chroma_location = AVCHROMA_LOC_LEFT;
		b.video_delay = 0;
		b.channel_layout = 0;
		b.channels = 0;
		b.sample_rate = 0;
		b.block_align = 0;
		b.frame_size = 0;
		b.initial_padding = 0;
		b.trailing_padding = 0;
		b.seek_preroll = 0;
		b.extradata_size = 32;
		b.extradata = new unsigned char[32];
		b.extradata[0] = 0;
		b.extradata[1] = 0;
		b.extradata[2] = 0;
		b.extradata[3] = 1;
		b.extradata[4] = 103;
		b.extradata[5] = 66;
		b.extradata[6] = 128;
		b.extradata[7] = 51;
		b.extradata[8] = 139;
		b.extradata[9] = 149;
		b.extradata[10] = 0;
		b.extradata[11] = 86;
		b.extradata[12] = 0;
		b.extradata[13] = 138;
		b.extradata[14] = 208;
		b.extradata[15] = 128;
		b.extradata[16] = 0;
		b.extradata[17] = 3;
		b.extradata[18] = 132;
		b.extradata[19] = 0;
		b.extradata[20] = 0;
		b.extradata[21] = 175;
		b.extradata[22] = 200;
		b.extradata[23] = 66;
		b.extradata[24] = 0;
		b.extradata[25] = 0;
		b.extradata[26] = 0;
		b.extradata[27] = 1;
		b.extradata[28] = 104;
		b.extradata[29] = 222;
		b.extradata[30] = 56;
		b.extradata[31] = 128;

		_outputCodec = avcodec_alloc_context3(decode);
		if (avcodec_parameters_to_context(_outputCodec, &b) < 0) {
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
	_inputTimeBase.den = 90000;
	_inputTimeBase.num = 1;
	//_inputTimeBase = inputHandler.Stream()->time_base;
	_frameIndex = 1;
	_frameSpan = 40;
	_ptsBase = 3600;
	//_frameSpan = inputHandler.FrameSpan();
	//_ptsBase = (inputHandler.Stream()->time_base.den) / inputHandler.Stream()->time_base.num / (1000 / inputHandler.FrameSpan());
	return ChannelStatus::Normal;
}

ChannelStatus FFmpegOutput::Init(int channelIndex, const string& outputUrl, const string& inputUrl,int frameCount)
{
	FFmpegInput handler;
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
}

void FFmpegOutput::PushRtmpPacket(AVPacket* packet, unsigned int frameIndex, long long duration)
{
	if (_outputFormat != NULL)
	{
		packet->pts = duration + frameIndex * _frameSpan;
		//packet->pts = packet->pts == AV_NOPTS_VALUE ? duration + frameIndex * _frameSpan : duration + av_rescale_q_rnd(packet->pts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		packet->dts=packet->pts;
	/*	if (packet->dts!= AV_NOPTS_VALUE)
		{
			packet->dts = duration + av_rescale_q_rnd(packet->dts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		}*/
		//packet->duration = av_rescale_q(packet->duration, _inputTimeBase, _outputStream->time_base);
		packet->duration = _frameSpan;
		packet->pos = -1;
		av_write_frame(_outputFormat, packet);
	}
}

bool FFmpegOutput::PushMp4Packet(unsigned char* data, int size)
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