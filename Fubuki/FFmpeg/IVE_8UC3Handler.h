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
		IVE_8UC3Handler(int count = 100);

		/**
		* @brief: д����Ƶ֡
		* @param: ive 8uc3�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: packetIndex ��Ƶ֡���
		*/
		void HandleFrame(unsigned char* ive, int width, int height, long long packetIndex);
		
		/**
		* @brief: 8uc3תbmp base64�ַ���
		* @param: ive 8uc3�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: base64 ���ڴ��base64�ַ���
		*/
		void ToBase64String(unsigned char* ive, int width, int height, std::string* base64);

	private:

		/**
		* @brief: 8uc3תbmp�ֽ���
		* @param: ive 8uc3�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		*/
		unsigned char* IveToBmp(unsigned char* ive, int width, int height);

		//bmp�ļ�ͷ����
		static const int HeaderSize;
		//��ǰд������
		int _index;
		//д������
		int _count;
	};
}


