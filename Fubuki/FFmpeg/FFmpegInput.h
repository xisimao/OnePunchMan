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
	//视频输入
	class FFmpegInput
	{
	public:
		/**
		* @brief: 构造函数
		*/
		FFmpegInput();

		/**
		* @brief: 初始化ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* @brief: 卸载ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* @brief: 初始化视频输入
		* @param: inputUrl 视频输入地址
		* @return: 视频状态
		*/
		bool Init(const std::string& inputUrl);

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


