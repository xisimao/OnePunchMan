#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegInput.h"

namespace OnePunchMan
{
	//��Ƶ���
	class RtmpOutput
	{
	public:
		/**
		* @brief: ���캯��
		*/
		RtmpOutput();

		/**
		* @brief: ��ʼ����Ƶ���
		* @param: outputUrl ��Ƶ�����ַ
		* @param: inputHandler ��Ƶ���������
		* @return: ��Ƶ״̬
		*/
		ChannelStatus Init(const std::string& outputUrl,const FFmpegInput* inputHandler);

		/**
		* @brief: ж����Ƶ���
		*/
		void Uninit();

		/**
		* @brief: ����rmtp��Ƶ��
		* @param: packet ��Ƶ��
		* @param: frameIndex ��Ƶ֡���
		* @param: duration ��ǰ��Ƶ��ʱ������
		*/
		void PushRtmpPacket(AVPacket* packet, unsigned int frameIndex,long long duration);
		
	private:
		//��Ƶ���
		AVFormatContext* _outputFormat;
		//��Ƶ�����
		AVStream* _outputStream;
		//�������
		AVCodecContext* _outputCodec;
		//�����׼���ʱ��
		long long _ptsBase;

	};
}


