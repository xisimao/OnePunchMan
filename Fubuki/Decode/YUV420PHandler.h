#pragma once
#include <stdio.h>

#include "StringEx.h"

namespace Fubuki
{
	//yuv420p文件写入
	class YUV420PHandler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: count 写入帧的总数
		*/
		YUV420PHandler(int count = 3);

		/**
		* @brief: 写入视频帧
		* @param: yuv yuv字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: packetIndex 视频帧序号
		*/
		void HandleFrame(unsigned char* yuv, int width,int height,int packetIndex);

	private:
		//当前写入数量
		int _index;
		//写入总量
		int _count;

	};
}


