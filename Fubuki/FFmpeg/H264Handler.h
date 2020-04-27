#pragma once
#include <stdio.h>

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
		* @brief: ��������
		*/
		~H264Handler();

		/**
		* @brief: ������Ƶ֡
		* @param: frame ��Ƶ֡�ֽ���
		* @param: size ��Ƶ֡�ֽ�������
		*/
		void HandleFrame(unsigned char* frame, int size);

	private:
		//��ǰд������
		int _index;
		//д������
		int _count;
		//h264�ļ�
		FILE* _h264File;

	};
}
