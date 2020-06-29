#pragma once
#include <stdio.h>
#include <string.h>

#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	//8uc3תbmp
	class IVEHandler
	{
	public:
		/**
		* @brief: ���캯��
		* @param: count д��֡������
		*/
		IVEHandler(int count = 3);

		/**
		* @brief: д����Ƶ֡
		* @param: ive 8uc3�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: frameIndex ��Ƶ֡���
		*/
		void HandleFrame(const unsigned char* ive, int width, int height, unsigned int frameIndex);
		
	private:

		//bmp�ļ�ͷ����
		static const int HeaderSize;
		//��ǰд������
		int _index;
		//д������
		int _count;
	};
}


