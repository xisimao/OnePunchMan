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
		*/
		FFmpegOutput();

		/**
		* @brief: ��ʼ����Ƶ���
		* @param: outputUrl ��Ƶ�����ַ
		* @param: inputHandler ��Ƶ���������
		* @return: ��Ƶ״̬
		*/
		ChannelStatus Init(const std::string& outputUrl,const FFmpegInput& inputHandler);
		/**
		* @brief: ��ʼ����Ƶ����
		* @param: channelIndex ͨ�����
		* @param: outputUrl ��Ƶ�����ַ
		* @param: inputUrl ��Ƶ�����ַ
		* @param: frameCount �����Ƶ֡����
		* @return: ��Ƶ״̬
		*/
		ChannelStatus Init(int channelIndex,const std::string& outputUrl,const std::string& inputUrl,int frameCount);
		
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
		
		/**
		* @brief: ����mp4��Ƶ��
		* @param: data ��Ƶ���ֽ���
		* @param: size ��Ƶ���ֽ�������
		*/
		bool PushMp4Packet(unsigned char* data,int size);

	private:
		//��Ƶ���
		AVFormatContext* _outputFormat;
		//��Ƶ�����
		AVStream* _outputStream;
		//�������
		AVCodecContext* _outputCodec;
		//������Ƶʱ�����
		AVRational _inputTimeBase;
		//ͨ�����
		int _channelIndex;
		//��ǰ�Ѿ����������
		int _frameIndex;
		//��Ҫ�������֡��
		int _frameCount;
		//������Ƶ��֡���ʱ��(����)
		int _frameSpan;
		//��������Ƶ��ʱ��������������֡��Ƶ��ʱ����
		int _ptsBase;
	
	};
}


