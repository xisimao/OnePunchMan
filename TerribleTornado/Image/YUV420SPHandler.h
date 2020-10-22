#pragma once
#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	class YUV420SPHandler
	{
	public:
		/**
		* ���캯��
		* @param count д��֡������
		*/
		YUV420SPHandler(int count = 3);

		/**
		* д����Ƶ֡
		* @param yuv yuv�ֽ���
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param frameIndex ��Ƶ֡���
		*/
		void HandleFrame(unsigned char* yuv, int width, int height, unsigned int frameIndex);

	private:
		//��ǰд������
		int _index;
		//д������
		int _count;
	};

}


