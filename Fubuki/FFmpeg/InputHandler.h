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

	//视频输入
	class InputHandler
	{
	public:
		/**
		* @brief: 构造函数
		*/
		InputHandler();

		/**
		* @brief: 初始化视频输入
		* @param: inputUrl 视频输入地址
		* @return: 视频状态
		*/
		ChannelStatus Init(const std::string& inputUrl);

		/**
		* @brief: 卸载视频输入
		*/
		void Uninit();

		/**
		* @brief: 获取视频输入
		* @return: 视频输入
		*/
		AVFormatContext* FormatContext();

		/**
		* @brief: 获取视频流
		* @return: 视频流
		*/
		AVStream* Stream() const;

		/**
		* @brief: 获取视频流序号
		* @return: 视频流序号
		*/
		int VideoIndex() const;

		/**
		* @brief: 获取输入视频宽度
		* @return: 输入视频宽度
		*/
		int SourceWidth() const;

		/**
		* @brief: 获取输入视频高度
		* @return: 输入视频高度
		*/
		int SourceHeight() const;

		/**
		* @brief: 获取输入视频帧间隔时长(毫秒)
		* @return: 输入视频帧间隔时长(毫秒)
		*/
		unsigned char FrameSpan() const;

	private:
		//视频输入相关
		std::string _inputUrl;
		//输入视频
		AVFormatContext* _inputFormat;
		//视频流
		AVStream* _inputStream;
		//视频流序号
		int _inputVideoIndex;
		//视频源宽度
		int _sourceWidth;
		//视频源高度
		int _sourceHeight;
		//输入视频帧间隔时长(毫秒)
		unsigned char _frameSpan;
	};
}


