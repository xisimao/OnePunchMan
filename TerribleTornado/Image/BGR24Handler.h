#pragma once
#include <stdio.h>

#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	//bgr24תbmp
	class BGR24Handler
	{
	public:
		/**
		* @brief: ���캯��
		* @param: count д��֡������
		*/
		BGR24Handler(int count=3);

		/**
		* @brief: д��bmp�ļ�
		* @param: bgr24 bgr24��ʽ���ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: frameIndex ֡���
		*/
		void HandleFrame(unsigned char* bgr24, int width, int height, unsigned int frameIndex);

	private:
		//��ǰд������
		int _index;
		//д������
		int _count;
	};
}


