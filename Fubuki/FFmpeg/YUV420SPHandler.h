#pragma once
#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	class YUV420SPHandler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: count 写入帧的总数
		*/
		YUV420SPHandler(int count = 3);

		/**
		* @brief: 写入视频帧
		* @param: yuv yuv字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: frameIndex 视频帧序号
		*/
		void HandleFrame(unsigned char* yuv, int width, int height, int frameIndex);

	private:
		//当前写入数量
		int _index;
		//写入总量
		int _count;
	};

}


