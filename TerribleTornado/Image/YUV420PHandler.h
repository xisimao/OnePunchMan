#pragma once
#include <stdio.h>

#include "Path.h"
#include "StringEx.h"

namespace OnePunchMan
{
	//yuv420p�ļ�д��
	class YUV420PHandler
	{
	public:
		/**
		* @brief: ���캯��
		* @param: count д��֡������
		*/
		YUV420PHandler(int count = 3);

		/**
		* @brief: д����Ƶ֡
		* @param: yuv yuv�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: frameIndex ��Ƶ֡���
		*/
		void HandleFrame(unsigned char* yuv, int width,int height, unsigned int frameIndex);

	private:
		//��ǰд������
		int _index;
		//д������
		int _count;

	};
}


