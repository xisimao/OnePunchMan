#include "FFmpegOutput.h"

using namespace std;
using namespace OnePunchMan;

FFmpegOutput::FFmpegOutput()
	: _outputUrl(),_outputFormat(NULL), _outputStream(NULL), _outputCodec(NULL)
	, _iFrameIndex(0),_iFrameCount(0), _frameIndex(0), _frameSpan(0)
{

}

bool FFmpegOutput::Init(const std::string& outputUrl,int iFrameCount)
{
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
	return Init(outputUrl, iFrameCount, &avParas);
}

bool FFmpegOutput::Init(const std::string& outputUrl, int iFrameCount, unsigned char* extraData, int extraDataSize)
{
	AVCodecParameters parameters;
	parameters.codec_type = AVMEDIA_TYPE_VIDEO;
	parameters.codec_id = AV_CODEC_ID_H264;
	parameters.codec_tag = 0;
	parameters.format = 0;
	parameters.bit_rate = 0;
	parameters.bits_per_coded_sample = 0;
	parameters.bits_per_raw_sample = 8;
	parameters.profile = 66;
	parameters.level = 51;
	parameters.width = 1920;
	parameters.height = 1080;
	parameters.sample_aspect_ratio.num = 0;
	parameters.sample_aspect_ratio.den = 1;
	parameters.field_order = AV_FIELD_PROGRESSIVE;
	parameters.color_range = AVCOL_RANGE_UNSPECIFIED;
	parameters.color_primaries = AVCOL_PRI_UNSPECIFIED;
	parameters.color_trc = AVCOL_TRC_UNSPECIFIED;
	parameters.color_space = AVCOL_SPC_UNSPECIFIED;
	parameters.chroma_location = AVCHROMA_LOC_LEFT;
	parameters.video_delay = 0;
	parameters.channel_layout = 0;
	parameters.channels = 0;
	parameters.sample_rate = 0;
	parameters.block_align = 0;
	parameters.frame_size = 0;
	parameters.initial_padding = 0;
	parameters.trailing_padding = 0;
	parameters.seek_preroll = 0;
	parameters.extradata_size = extraDataSize;
	parameters.extradata = extraData;
	return Init(outputUrl, iFrameCount, &parameters);

}

bool FFmpegOutput::Init(const std::string& outputUrl, int iFrameCount, const AVCodecParameters* parameters)
{
	_outputUrl = outputUrl;
	_iFrameCount = iFrameCount;
	if (_outputFormat != NULL)
	{
		LogPool::Warning(LogEvent::Encode, "已经初始化了输出文件", _outputUrl);
		return true;
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
	LogPool::Information(LogEvent::Encode, "初始化输出视频:", _outputUrl, "I帧输出数量:", _iFrameCount);
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
		LogPool::Information("结束输出视频:", _outputUrl, "共输出:", _frameIndex, "帧");
	}

}

bool FFmpegOutput::Finished()
{
	return _outputFormat == NULL;
}

void FFmpegOutput::WritePacket(const unsigned char* data,int size, FrameType frameType)
{
	if (_outputFormat == NULL)
	{
		return;
	}
	else
	{
		if (frameType == FrameType::I)
		{
			_iFrameIndex += 1;
			if (_iFrameCount!=-1&&_iFrameIndex > _iFrameCount)
			{
				Uninit();
				return;
			}
		}
		AVPacket* packet = av_packet_alloc();
		unsigned char* temp = (unsigned char*)av_malloc(size);
		memcpy(temp, data, size);
		av_packet_from_data(packet, temp, size);
		packet->flags = frameType == FrameType::I ? 1 : 0;
		packet->pts = _frameIndex * _frameSpan;
		packet->dts = packet->pts;
		packet->pos = -1;
		packet->duration = _frameSpan;
		av_interleaved_write_frame(_outputFormat, packet);
		av_packet_free(&packet);
		_frameIndex += 1;
	}
}

void FFmpegOutput::WritePacket(AVPacket* packet)
{
	if (_outputFormat != NULL)
	{
		//packet->pts = packet->pts == AV_NOPTS_VALUE ? duration + frameIndex * _frameSpan : duration + av_rescale_q_rnd(packet->pts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//if (packet->dts!= AV_NOPTS_VALUE)
		//{
		//	packet->dts = duration + av_rescale_q_rnd(packet->dts, _inputTimeBase, _outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//}
		//packet->duration = av_rescale_q(packet->duration, _inputTimeBase, _outputStream->time_base);

		packet->pts = _frameIndex * _frameSpan;
		packet->dts = packet->pts;
		packet->duration = _frameSpan;
		packet->pos = -1;
		packet->duration = _frameSpan;
		av_write_frame(_outputFormat, packet);
		_frameIndex += 1;
	}
}
