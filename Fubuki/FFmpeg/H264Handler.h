#pragma once
#include <stdio.h>

#include "StringEx.h"

namespace OnePunchMan
{
	//h264�ļ�д��
	class H264Handler
	{
	public:
		/**
		* @brief: ���캯��
		* @param: count д��֡������
		*/
		H264Handler(int count=100);

		/**
		* @brief: ������Ƶ֡
		* @param: frame ��Ƶ֡�ֽ���
		* @param: size ��Ƶ֡�ֽ�������
		* @param: frameIndex ��Ƶ֡���
		*/
		void HandleFrame(unsigned char* frame, int size);

	private:
		//��ǰд������
		int _frameIndex;
		//д������
		int _count;
	};
}
