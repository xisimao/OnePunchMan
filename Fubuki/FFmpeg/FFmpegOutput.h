#pragma once
#include "LogPool.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace OnePunchMan
{
	//h264֡����
	enum class FrameType
	{
		P = 1,
		I = 5,
		SPS = 7,
		PPS = 8
	};
	//��Ƶ���
	class FFmpegOutput
	{
	public:
		/**
		* @brief: ���캯��
		*/
		FFmpegOutput();

		/**
		* @brief: ��ʼ����Ƶ���,�����޷���ȡsps��pps��ffmpegҲ�޷���ȡ
		* @param: outputUrl �����ַ
		* @return: ��ʼ�����
		*/
		bool Init(const std::string& outputUrl, int iFrameCount);

		/**
		* @brief: ��ʼ����Ƶ��������ڿ��Ի�ȡ��sps��pps����
		* @param: outputUrl �����ַ
		* @param: iFrameCount ���i֡������
		* @param: extraData sps+pps�ֽ���
		* @param: extraDataSize sps+pps�ֽ�������
		* @return: ��ʼ�����
		*/
		bool Init(const std::string& outputUrl, int iFrameCount, unsigned char* extraData,int extraDataSize);

		/**
		* @brief: ��ʼ����Ƶ��������ڿ��Ի�ȡ��sps��pps����
		* @param: outputUrl �����ַ
		* @param: iFrameCount ���i֡������
		* @param: parameters ����������
		* @return: ��ʼ�����
		*/
		bool Init(const std::string& outputUrl, int iFrameCount, const AVCodecParameters* parameters);

		/**
		* @brief: �����Ƶ��
		* @param: data h264��Ƶ���ֽ���
		* @param: size h264��Ƶ������
		* @param: frameType ֡����
		*/
		void WritePacket(const unsigned char* data, int size, FrameType frameType);

		/**
		* @brief: �����Ƶ��
		* @param: packet ��Ƶ��
		*/
		void WritePacket(AVPacket* packet);

		/**
		* @brief: ��ȡ�����Ƶ�Ƿ��Ѿ�����
		* @return: �Ѿ�����ʱ����true
		*/
		bool Finished();

		/**
		* @brief: ������Ƶ���
		*/
		void Uninit();

	private:
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


