#include "RtmpOutput.h"

using namespace std;
using namespace OnePunchMan;

RtmpOutput::RtmpOutput()
	:_outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL),_ptsBase(0)
{
}

ChannelStatus RtmpOutput::Init(const string& outputUrl, const FFmpegInput* inputHandler)
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

		AVCodecParameters avParas;
		avParas.codec_type = AVMEDIA_TYPE_VIDEO;
		avParas.codec_id = AV_CODEC_ID_H264;
		avParas.codec_tag = 0;
		avParas.format = 0;
		avParas.bit_rate = 0;
		avParas.bits_per_coded_sample = 0;
		avParas.bits_per_raw_sample = 8;
		avParas.profile = 66;
		avParas.level = 51;
		avParas.width = 1920;
		avParas.height = 1080;
		avParas.sample_aspect_ratio.num = 0;
		avParas.sample_aspect_ratio.den = 1;
		avParas.field_order = AV_FIELD_PROGRESSIVE;
		avParas.color_range = AVCOL_RANGE_UNSPECIFIED;
		avParas.color_primaries = AVCOL_PRI_UNSPECIFIED;
		avParas.color_trc = AVCOL_TRC_UNSPECIFIED;
		avParas.color_space = AVCOL_SPC_UNSPECIFIED;
		avParas.chroma_location = AVCHROMA_LOC_LEFT;
		avParas.video_delay = 0;
		avParas.channel_layout = 0;
		avParas.channels = 0;
		avParas.sample_rate = 0;
		avParas.block_align = 0;
		avParas.frame_size = 0;
		avParas.initial_padding = 0;
		avParas.trailing_padding = 0;
		avParas.seek_preroll = 0;
		avParas.extradata_size = 32;
		avParas.extradata = new unsigned char[32];
		avParas.extradata[0] = 0;
		avParas.extradata[1] = 0;
		avParas.extradata[2] = 0;
		avParas.extradata[3] = 1;
		avParas.extradata[4] = 103;
		avParas.extradata[5] = 66;
		avParas.extradata[6] = 128;
		avParas.extradata[7] = 51;
		avParas.extradata[8] = 139;
		avParas.extradata[9] = 149;
		avParas.extradata[10] = 0;
		avParas.extradata[11] = 86;
		avParas.extradata[12] = 0;
		avParas.extradata[13] = 138;
		avParas.extradata[14] = 208;
		avParas.extradata[15] = 128;
		avParas.extradata[16] = 0;
		avParas.extradata[17] = 3;
		avParas.extradata[18] = 132;
		avParas.extradata[19] = 0;
		avParas.extradata[20] = 0;
		avParas.extradata[21] = 175;
		avParas.extradata[22] = 200;
		avParas.extradata[23] = 66;
		avParas.extradata[24] = 0;
		avParas.extradata[25] = 0;
		avParas.extradata[26] = 0;
		avParas.extradata[27] = 1;
		avParas.extradata[28] = 104;
		avParas.extradata[29] = 222;
		avParas.extradata[30] = 56;
		avParas.extradata[31] = 128;
		AVCodecParameters* codecPara;
		if (inputHandler == NULL)
		{
			codecPara = &avParas;
		}
		else
		{
			codecPara = inputHandler->Stream()->codecpar;
		}

		_outputCodec = avcodec_alloc_context3(decode);
		if (avcodec_parameters_to_context(_outputCodec, codecPara) < 0) {
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
	if (inputHandler == NULL)
	{
		_ptsBase = 40;
	}
	else
	{
		_ptsBase = inputHandler->FrameSpan();
	}
	return ChannelStatus::Normal;
}

void RtmpOutput::Uninit()
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

void RtmpOutput::PushRtmpPacket(AVPacket* packet, unsigned int frameIndex, long long duration)
{
	if (_outputFormat != NULL)
	{
		//packet->pts = packet->pts == AV_NOPTS_VALUE ? duration + frameIndex * _frameSpan : duration + av_rescale_q_rnd(packet->pts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//if (packet->dts!= AV_NOPTS_VALUE)
		//{
		//	packet->dts = duration + av_rescale_q_rnd(packet->dts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//}
		//packet->duration = av_rescale_q(packet->duration, _inputTimeBase, _outputStream->time_base);

		packet->pts = duration + frameIndex * _ptsBase;
		packet->dts = packet->pts;
		packet->duration = _ptsBase;
		packet->pos = -1;
		av_write_frame(_outputFormat, packet);
	}
}