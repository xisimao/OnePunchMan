#pragma once
#include <stdio.h>

#include "StringEx.h"

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
		* @param: packetIndex ֡���
		*/
		void HandleFrame(unsigned char* bgr24, int width, int height,int packetIndex);

		/**
		* @brief: д��jpg�ļ�
		* @param: jpg jpg���ֽ���
		* @param: size jpg���ֽ�������
		* @param: packetIndex ֡���
		*/
		void WriteJpg(unsigned char* jpg, int size, int packetIndex);

	private:
		//��ǰд������
		int _index;
		//д������
		int _count;
	};
}


