#pragma once
#include <stdio.h>

namespace OnePunchMan
{
	//h264文件写入
	class H264Handler
	{
	public:
		/**
		* @brief: 构造函数
		* @param: count 写入帧的总数
		*/
		H264Handler(int count=100);

		/**
		* @brief: 析构函数
		*/
		~H264Handler();

		/**
		* @brief: 处理视频帧
		* @param: frame 视频帧字节流
		* @param: size 视频帧字节流长度
		*/
		void HandleFrame(unsigned char* frame, int size);

	private:
		//当前写入数量
		int _index;
		//写入总数
		int _count;
		//h264文件
		FILE* _h264File;

	};
}
