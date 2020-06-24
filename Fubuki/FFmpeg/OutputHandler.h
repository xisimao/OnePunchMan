#pragma once
#include <string>

#include "LogPool.h"
#include "InputHandler.h"

namespace OnePunchMan
{
	enum OutputType
	{
		None,
		Rtmp,
		Mp4
	};
	class OutputHandler
	{
	public:
		OutputHandler();

		ChannelStatus Init(const std::string& outputUrl,const InputHandler& inputHandler);

		ChannelStatus Init(int channelIndex,const std::string& outputUrl,const std::string& inputUrl,int frameCount);

		int ChannelIndex();

		bool Finished();

		void Uninit();

		void PushPacket(AVPacket* packet, unsigned int frameIndex,long long duration);

		void PushPacket(unsigned char* data,int size);

	private:
		//视频输出相关
		AVFormatContext* _outputFormat;
		AVStream* _outputStream;
		AVCodecContext* _outputCodec;
		AVRational _inputTimeBase;
		OutputType _type;
		int _channelIndex;
		int _frameIndex;
		int _frameSpan;
		int _frameCount;
		int _ptsBase;
	};
}


