#pragma once
#include "FrameChannel.h"
#include "H264Handler.h"
#include "YUV420PHandler.h"
#include "BGR24Handler.h"

extern "C"
{
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace Fubuki
{
	class FFmpegChannel :public FrameChannel
	{
	public:
		FFmpegChannel();

	protected:
		int InitCore();

		void UninitCore();

		bool Decode(const AVPacket* packet, int packetIndex);

	private:
		AVCodecContext* _decodeContext;
		AVFrame* _yuvFrame;
		AVFrame* _bgrFrame;
		uint8_t* _bgrBuffer;
		SwsContext* _bgrSwsContext;

		H264Handler* _h264Handler;
		YUV420PHandler* _yuvHandler;
		BGR24Handler* _bgrHandler;
	
	};

}

