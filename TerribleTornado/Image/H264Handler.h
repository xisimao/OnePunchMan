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
		H264Handler(int channelIndex);

		/**
		* @brief: ������Ƶ֡
		* @param: frame ��Ƶ֡�ֽ���
		* @param: size ��Ƶ֡�ֽ�������
		*/
		void HandleFrame(unsigned char* frame, int size);

		/**
		* @brief: ������Ƶ֡
		* @param: frame ��Ƶ֡�ֽ���
		* @param: size ��Ƶ֡�ֽ�������
		*/
		void HandleFrame(int frameIndex, unsigned char* frame, int size);

		void Close();

	private:
		//h264�ļ�
		FILE* _h264File;
		//ͨ�����
		int _channelIndex;
	};
}
