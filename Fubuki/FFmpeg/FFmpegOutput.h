#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegInput.h"

namespace OnePunchMan
{
	//视频输出
	class FFmpegOutput
	{
	public:
		/**
		* @brief: 构造函数
		*/
		FFmpegOutput();

		/**
		* @brief: 初始化视频输出
		* @param: outputUrl 视频输出地址
		* @param: inputHandler 视频输入操作类
		* @return: 视频状态
		*/
		ChannelStatus Init(const std::string& outputUrl,const FFmpegInput& inputHandler);
		/**
		* @brief: 初始化视频输入
		* @param: channelIndex 通道序号
		* @param: outputUrl 视频输出地址
		* @param: inputUrl 视频输入地址
		* @param: frameCount 输出视频帧总数
		* @return: 视频状态
		*/
		ChannelStatus Init(int channelIndex,const std::string& outputUrl,const std::string& inputUrl,int frameCount);
		
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
		
		/**
		* @brief: 推送mp4视频包
		* @param: data 视频包字节流
		* @param: size 视频包字节流长度
		*/
		bool PushMp4Packet(unsigned char* data,int size);

	private:
		//视频输出
		AVFormatContext* _outputFormat;
		//视频输出流
		AVStream* _outputStream;
		//解码相关
		AVCodecContext* _outputCodec;
		//输入视频时间基数
		AVRational _inputTimeBase;
		//通道序号
		int _channelIndex;
		//当前已经输出的数量
		int _frameIndex;
		//需要输出的总帧数
		int _frameCount;
		//输入视频两帧间隔时长(毫秒)
		int _frameSpan;
		//以输入视频的时间基数换算过的两帧视频的时间间隔
		int _ptsBase;
	
	};
}


