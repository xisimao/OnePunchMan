#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegInput.h"

namespace OnePunchMan
{
	//��Ƶ���
	class Mp4Output
	{
	public:
		/**
		* @brief: ���캯��
		* @param: outputUrl ����ļ���ַ
		* @param: iFrameCount ��Ҫ�����i֡������
		*/
		Mp4Output(const std::string& outputUrl,int iFrameCount);

		/**
		* @brief: ��ʼ����Ƶ���
		* @param: extradata sps+pps
		* @param: extradataSize sps+pps����
		* @return: ��ʼ�����
		*/
		bool Init(const AVCodecParameters* parameters);

		/**
		* @brief: ���캯��
		* @param: packet h264��Ƶ��
		* @param: frameType ֡����
		*/
		void WritePacket(AVPacket* packet, int frameType);

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


