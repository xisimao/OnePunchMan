#pragma once
#include <stdio.h>
#include <string.h>

#include "StringEx.h"

namespace OnePunchMan
{
	//8uc3转bmp
	class IVE_8UC3Handler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: count 写入帧的总数
		*/
		IVE_8UC3Handler(int count = 100);

		/**
		* @brief: 写入视频帧
		* @param: ive 8uc3字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: packetIndex 视频帧序号
		*/
		void HandleFrame(unsigned char* ive, int width, int height, long long packetIndex);
		
		/**
		* @brief: 8uc3转bmp base64字符串
		* @param: ive 8uc3字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: base64 用于存放base64字符串
		*/
		void ToBase64String(unsigned char* ive, int width, int height, std::string* base64);

	private:

		/**
		* @brief: 8uc3转bmp字节流
		* @param: ive 8uc3字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		*/
		unsigned char* IveToBmp(unsigned char* ive, int width, int height);

		//bmp文件头长度
		static const int HeaderSize;
		//当前写入数量
		int _index;
		//写入总量
		int _count;
	};
}


