#pragma once
#include <stdio.h>

#include "StringEx.h"

namespace Fubuki
{
	//8uc3转bmp
	class IVE_8UC3Handler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: count 写入帧的总数
		*/
		IVE_8UC3Handler(int count = 3);

		/**
		* @brief: 写入视频帧
		* @param: ive 8uc3字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: packetIndex 视频帧序号
		*/
		void HandleFrame(unsigned char* ive, int width, int height, int packetIndex);

	private:
		//当前写入数量
		int _index;
		//写入总量
		int _count;
	};
}


