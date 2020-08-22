#pragma once
#include <stdio.h>

#include "StringEx.h"

namespace OnePunchMan
{
	//h264文件写入
	class H264Handler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: channelIndex 视频序号
		* @param: count 写入帧的总数
		*/
		H264Handler(int channelIndex);

		/**
		* @brief: 处理视频帧
		* @param: frame 视频帧字节流
		* @param: size 视频帧字节流长度
		*/
		void HandleFrame(unsigned char* frame, int size);

		/**
		* @brief: 处理视频帧
		* @param: frame 视频帧字节流
		* @param: size 视频帧字节流长度
		*/
		void HandleFrame(int frameIndex, unsigned char* frame, int size);

		void Close();

	private:
		//h264文件
		FILE* _h264File;
		//通道序号
		int _channelIndex;
	};
}
