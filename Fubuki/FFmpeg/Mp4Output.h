#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegInput.h"

namespace OnePunchMan
{
	//视频输出
	class Mp4Output
	{
	public:
		/**
		* @brief: 构造函数
		* @param: outputUrl 输出文件地址
		* @param: iFrameCount 需要输出的i帧的数量
		*/
		Mp4Output(const std::string& outputUrl,int iFrameCount);

		/**
		* @brief: 初始化视频输出
		* @param: extradata sps+pps
		* @param: extradataSize sps+pps长度
		* @return: 初始化结果
		*/
		bool Init(const AVCodecParameters* parameters);

		/**
		* @brief: 构造函数
		* @param: packet h264视频包
		* @param: frameType 帧类型
		*/
		void WritePacket(AVPacket* packet, int frameType);

		/**
		* @brief: 获取输出视频是否已经结束
		* @return: 已经结束时返回true
		*/
		bool Finished();

	private:
		/**
		* @brief: 结束视频输出
		*/
		void Uninit();

		//以输入视频的时间基数换算过的两帧视频的时间间隔
		const static long long PtsBase;

		//输出文件地址
		std::string _outputUrl;
		//视频输出
		AVFormatContext* _outputFormat;
		//视频输出流
		AVStream* _outputStream;
		//解码相关
		AVCodecContext* _outputCodec;

		//当前i帧的序号
		int _iFrameIndex;
		//需要输出i帧的数量
		int _iFrameCount;

		//当前已经输出的帧数量
		int _frameIndex;

		//两帧时间间隔时间
		int _frameSpan;
	};
}


