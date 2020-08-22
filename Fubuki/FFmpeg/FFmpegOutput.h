#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegInput.h"

namespace OnePunchMan
{
	//��Ƶ���
	class FFmpegOutput
	{
	public:
		/**
		* @brief: ���캯��
		* @param: outputUrl ����ļ���ַ
		* @param: iFrameCount ��Ҫ�����i֡������
		*/
		FFmpegOutput(const std::string& outputUrl,int iFrameCount);

		/**
		* @brief: ��ʼ����Ƶ���
		* @param: extradata sps+pps
		* @param: extradataSize sps+pps����
		* @return: ��ʼ�����
		*/
		bool Init(const AVCodecParameters* parameters);

		/**
		* @brief: ���캯��
		* @param: data h264��Ƶ���ֽ���
		* @param: size h264��Ƶ������
		* @param: frameType ֡����
		*/
		void WritePacket(const unsigned char* data, int size, int frameType);

		/**
		* @brief: ��ȡ�����Ƶ�Ƿ��Ѿ�����
		* @return: �Ѿ�����ʱ����true
		*/
		bool Finished();

	private:
		/**
		* @brief: ������Ƶ���
		*/
		void Uninit();

		//��������Ƶ��ʱ��������������֡��Ƶ��ʱ����
		const static long long PtsBase;

		//����ļ���ַ
		std::string _outputUrl;
		//��Ƶ���
		AVFormatContext* _outputFormat;
		//��Ƶ�����
		AVStream* _outputStream;
		//�������
		AVCodecContext* _outputCodec;

		//��ǰi֡�����
		int _iFrameIndex;
		//��Ҫ���i֡������
		int _iFrameCount;

		//��ǰ�Ѿ������֡����
		int _frameIndex;

		//��֡ʱ����ʱ��
		int _frameSpan;
	};
}


