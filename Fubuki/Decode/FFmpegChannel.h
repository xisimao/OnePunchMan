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
	//FFmpeg解码
	class FFmpegChannel :public FrameChannel
	{
	public:
		/**
		* @brief: 构造函数
		* @param: inputUrl 输入url
		* @param: outputUrl 输出url
		* @param: loop 是否循环播放
		*/
		FFmpegChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop);

	protected:
		bool InitDecoder();

		void UninitDecoder();

		bool Decode(const AVPacket* packet, int packetIndex);

	private:
		//解码相关
		AVCodecContext* _decodeContext;
		//解码后的yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		uint8_t* _bgrBuffer;
		SwsContext* _bgrSwsContext;

		//h264写入
		H264Handler* _h264Handler;
		//yuv420p写入
		YUV420PHandler* _yuvHandler;
		//bgr24写入
		BGR24Handler* _bgrHandler;
	
	};

}

