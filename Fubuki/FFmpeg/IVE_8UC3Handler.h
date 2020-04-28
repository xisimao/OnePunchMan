#pragma once
#include <stdio.h>
#include <string.h>

#include "StringEx.h"

namespace OnePunchMan
{
	//8uc3תbmp
	class IVE_8UC3Handler
	{
	public:
		/**
		* @brief: ���캯��
		* @param: count д��֡������
		*/
		IVE_8UC3Handler(int count = 3);

		/**
		* @brief: д����Ƶ֡
		* @param: ive 8uc3�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: packetIndex ��Ƶ֡���
		*/
		void HandleFrame(unsigned char* ive, int width, int height, long long packetIndex);
		
	private:

		//bmp�ļ�ͷ����
		static const int HeaderSize;
		//��ǰд������
		int _index;
		//д������
		int _count;
	};
}


