#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegInput.h"

namespace OnePunchMan
{
	//视频输出
	class RtmpOutput
	{
	public:
		/**
		* @brief: 构造函数
		*/
		RtmpOutput();

		/**
		* @brief: 初始化视频输出
		* @param: outputUrl 视频输出地址
		* @param: inputHandler 视频输入操作类
		* @return: 视频状态
		*/
		ChannelStatus Init(const std::string& outputUrl,const FFmpegInput* inputHandler);

		/**
		* @brief: 卸载视频输出
		*/
		void Uninit();

		/**
		* @brief: 推送rmtp视频包
		* @param: packet 视频包
		* @param: frameIndex 视频帧序号
		* @param: duration 当前视频总时长基数
		*/
		void PushRtmpPacket(AVPacket* packet, unsigned int frameIndex,long long duration);
		
	private:
		//视频输出
		AVFormatContext* _outputFormat;
		//视频输出流
		AVStream* _outputStream;
		//解码相关
		AVCodecContext* _outputCodec;
		//解码基准间隔时间
		long long _ptsBase;

	};
}


