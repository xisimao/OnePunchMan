#pragma once
#include <stdio.h>

#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	//jpg文件写入
	class JPGHandler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: count 写入帧的总数
		*/
		JPGHandler(int count = 3);

		/**
		* @brief: 写入jpg文件
		* @param: jpgBuffer jpg格式的字节流
		* @param: jpgSize jgp字节流长度
		* @param: packetIndex 帧序号
		*/
		void HandleFrame(unsigned char* jpgBuffer, int jpgSize, int packetIndex);

	private:
		//当前写入数量
		int _index;
		//写入总数
		int _count;
	};
}


