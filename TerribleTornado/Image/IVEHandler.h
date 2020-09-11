#pragma once
#include <stdio.h>
#include <string.h>

#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	//8uc3转bmp
	class IVEHandler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: fileDir 写入的目录
		* @param: count 写入帧的总数
		*/
		IVEHandler(const std::string& fileDir,int count = 3);

		/**
		* @brief: 写入视频帧
		* @param: ive 8uc3字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: frameIndex 视频帧序号
		*/
		void HandleFrame(const unsigned char* ive, int width, int height, unsigned int frameIndex);
		
	private:
		//bmp文件头长度
		static const int HeaderSize;
		//写入的目录
		std::string _fileDir;
		//当前写入数量
		int _index;
		//写入总量
		int _count;
	};
}


