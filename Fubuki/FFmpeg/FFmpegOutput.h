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
	//h264帧类型
	enum class FrameType
	{
		P = 1,
		I = 5,
		SPS = 7,
		PPS = 8
	};
	//视频输出
	class FFmpegOutput
	{
	public:
		/**
		* @brief: 构造函数
		*/
		FFmpegOutput();

		/**
		* @brief: 初始化视频输出,用于无法获取sps和pps，ffmpeg也无法读取
		* @param: outputUrl 输出地址
		* @return: 初始化结果
		*/
		bool Init(const std::string& outputUrl, int iFrameCount);

		/**
		* @brief: 初始化视频输出，用于可以获取到sps和pps数据
		* @param: outputUrl 输出地址
		* @param: iFrameCount 输出i帧的数量
		* @param: extraData sps+pps字节流
		* @param: extraDataSize sps+pps字节流长度
		* @return: 初始化结果
		*/
		bool Init(const std::string& outputUrl, int iFrameCount, unsigned char* extraData,int extraDataSize);

		/**
		* @brief: 初始化视频输出，用于可以获取到sps和pps数据
		* @param: outputUrl 输出地址
		* @param: iFrameCount 输出i帧的数量
		* @param: parameters 输出解码参数
		* @return: 初始化结果
		*/
		bool Init(const std::string& outputUrl, int iFrameCount, const AVCodecParameters* parameters);

		/**
		* @brief: 输出视频包
		* @param: data h264视频包字节流
		* @param: size h264视频包长度
		* @param: frameType 帧类型
		*/
		void WritePacket(const unsigned char* data, int size, FrameType frameType);

		/**
		* @brief: 输出视频包
		* @param: packet 视频包
		*/
		void WritePacket(AVPacket* packet);

		/**
		* @brief: 获取输出视频是否已经结束
		* @return: 已经结束时返回true
		*/
		bool Finished();

		/**
		* @brief: 结束视频输出
		*/
		void Uninit();

	private:
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


