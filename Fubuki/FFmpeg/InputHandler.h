#pragma once
#include "LogPool.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace OnePunchMan
{
	//通道状态
	enum class ChannelStatus
	{
		//正常
		Normal = 1,
		//无法打开视频源
		InputError = 2,
		//Rtmp输出异常
		OutputError = 3,
		//解码器异常
		DecoderError = 4,
		//无法读取视频数据
		ReadError = 5,
		//解码错误
		DecodeError = 6,
		//准备循环播放
		ReadEOF_Restart = 7,
		//文件播放结束
		ReadEOF_Stop = 8,
		//正在初始化
		Init = 9,
		//网络异常(中心)
		Disconnect = 10,
		//通道不同步(中心)
		NotFoundChannel = 11,
		//车道不同步(中心)
		NotFoundLane = 12,
		//过多帧没有处理
		NotHandle = 13
	};

	class InputHandler
	{
	public:
		InputHandler();

		ChannelStatus Init(const std::string& inputUrl);
		
		void Uninit();

		AVFormatContext* FormatContext();
		AVStream* Stream() const;
		int VideoIndex() const;
		int SourceWidth() const;
		int SourceHeight() const;
		unsigned char FrameSpan() const;
	private:
		//视频输入相关
		std::string _inputUrl;
		AVFormatContext* _inputFormat;
		AVStream* _inputStream;
		int _inputVideoIndex;
		//视频源宽度
		int _sourceWidth;
		//视频源高度
		int _sourceHeight;
		unsigned char _frameSpan;
	};
}


