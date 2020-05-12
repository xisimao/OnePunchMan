#pragma once
#include <stdio.h>

#include "StringEx.h"
#include "Path.h"

namespace OnePunchMan
{
	//jpg�ļ�д��
	class JPGHandler
	{
	public:
		/**
		* @brief: ���캯��
		* @param: count д��֡������
		*/
		JPGHandler(int count = 3);

		/**
		* @brief: д��jpg�ļ�
		* @param: jpgBuffer jpg��ʽ���ֽ���
		* @param: jpgSize jgp�ֽ�������
		* @param: packetIndex ֡���
		*/
		void HandleFrame(unsigned char* jpgBuffer, int jpgSize, int packetIndex);

	private:
		//��ǰд������
		int _index;
		//д������
		int _count;
	};
}


