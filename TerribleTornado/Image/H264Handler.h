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
		* @param: channelIndex ��Ƶ���
		* @param: count д��֡������
		*/
		H264Handler(int channelIndex,int count=100);

		/**
		* @brief: ������Ƶ֡
		* @param: frame ��Ƶ֡�ֽ���
		* @param: size ��Ƶ֡�ֽ�������
		*/
		void HandleFrame(int frameIndex, unsigned char* frame, int size);

	private:
		//h264�ļ�
		FILE* _h264File;
		int _channelIndex;
		//��ǰд������
		int _frameIndex;
		//д������
		int _count;
	};
}
