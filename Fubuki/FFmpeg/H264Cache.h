#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegOutput.h"

namespace OnePunchMan
{
	//视频输出
	class H264Cache
	{
	public:
		/**
		* @brief: 构造函数
		* @param: channelIndex 通道序号
		*/
		H264Cache(int channelIndex);

		/**
		* @brief: 析构函数
		*/
		~H264Cache();

		/**
		* @brief: 推送h264视频包
		* @param: data 视频包字节流
		* @param: size 视频包字节流长度
		*/
		void PushPacket(unsigned char* data, int size);

		/**
		* @brief: 添加视频输出
		* @param: outputUrl 视频输出地址
		* @param: iFrameCount 输出I帧数量
		*/
		bool AddOutputUrl(const std::string& outputUrl, int iFrameCount);

		/**
		* @brief: 移除视频输出
		* @param: outputUrl 视频输出地址
		*/
		void RemoveOutputUrl(const std::string& outputUrl);

		/**
		* @brief: 检查视频输出是否完成
		* @param: outputUrl 视频输出地址
		* @return: 如果视频输出已经完成返回true并自动卸载,但是不会删除文件
		*/
		bool OutputFinished(const std::string& outputUrl);

		//I帧间隔
		const static int Gop;

		//输出队列最大数量
		const static int MaxOutputCount;

	private:

		class FrameItem
		{
		public:
			unsigned char* Data;
			unsigned int Size;
		};
		/**
		* @brief: 将h264视频包写入缓存
		* @param: data 视频包字节流
		* @param: size 视频包字节流长度
		* @param: frameIndex 视频包对应的帧序号
		* @param: frameType 视频包对应的帧类型
		* @return: 表示是否写入缓存成功
		*/
		bool WriteCache(unsigned char* data, int size, int frameIndex, FrameType frameType);

		//sps+pps最大长度
		const static int MaxExtraDataSize;
		//帧的最大长度
		const static int FrameSize;

		//通道序号
		int _channelIndex;

		//当前i帧的序号
		int _iFrameIndex;
		//当前p帧的序号
		int _pFrameIndex;
		//当前缓存帧的数量
		unsigned int _frameCount;

		//输出视频参数
		unsigned char* _extraData;
		//是否已经获取到sps帧
		int _spsSize;
		//是否已经获取到pps帧
		int _ppsSize;

		//帧缓存
		std::vector<FrameItem> _frameCache;

		//输出集合
		std::mutex _mutex;
		std::map<std::string, FFmpegOutput*> _outputItems;
	};
}


