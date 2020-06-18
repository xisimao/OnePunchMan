#pragma once
#include <stdio.h>

#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	//bgr24转bmp
	class BGR24Handler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: count 写入帧的总数
		*/
		BGR24Handler(int count=3);

		/**
		* @brief: 写入bmp文件
		* @param: bgr24 bgr24格式的字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: frameIndex 帧序号
		*/
		void HandleFrame(unsigned char* bgr24, int width, int height, unsigned int frameIndex);

	private:
		//当前写入数量
		int _index;
		//写入总数
		int _count;
	};
}


