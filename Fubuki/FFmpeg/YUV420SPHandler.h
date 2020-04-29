#pragma once
#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	class YUV420SPHandler
	{
	public:
		/**
		* @brief: ���캯��
		* @param: count д��֡������
		*/
		YUV420SPHandler(int count = 3);

		/**
		* @brief: д����Ƶ֡
		* @param: yuv yuv�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: packetIndex ��Ƶ֡���
		*/
		void HandleFrame(unsigned char* yuv, int width, int height, int packetIndex);

	private:
		//��ǰд������
		int _index;
		//д������
		int _count;
	};

}


