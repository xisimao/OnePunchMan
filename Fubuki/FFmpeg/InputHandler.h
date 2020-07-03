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
	//ͨ��״̬
	enum class ChannelStatus
	{
		//����
		Normal = 1,
		//�޷�����ƵԴ
		InputError = 2,
		//Rtmp����쳣
		OutputError = 3,
		//�������쳣
		DecoderError = 4,
		//�޷���ȡ��Ƶ����
		ReadError = 5,
		//�������
		DecodeError = 6,
		//׼��ѭ������
		ReadEOF_Restart = 7,
		//�ļ����Ž���
		ReadEOF_Stop = 8,
		//���ڳ�ʼ��
		Init = 9,
		//�����쳣(����)
		Disconnect = 10,
		//ͨ����ͬ��(����)
		NotFoundChannel = 11,
		//������ͬ��(����)
		NotFoundLane = 12,
		//����֡û�д���
		NotHandle = 13
	};

	//��Ƶ����
	class InputHandler
	{
	public:
		/**
		* @brief: ���캯��
		*/
		InputHandler();

		/**
		* @brief: ��ʼ����Ƶ����
		* @param: inputUrl ��Ƶ�����ַ
		* @return: ��Ƶ״̬
		*/
		ChannelStatus Init(const std::string& inputUrl);

		/**
		* @brief: ж����Ƶ����
		*/
		void Uninit();

		/**
		* @brief: ��ȡ��Ƶ����
		* @return: ��Ƶ����
		*/
		AVFormatContext* FormatContext();

		/**
		* @brief: ��ȡ��Ƶ��
		* @return: ��Ƶ��
		*/
		AVStream* Stream() const;

		/**
		* @brief: ��ȡ��Ƶ�����
		* @return: ��Ƶ�����
		*/
		int VideoIndex() const;

		/**
		* @brief: ��ȡ������Ƶ���
		* @return: ������Ƶ���
		*/
		int SourceWidth() const;

		/**
		* @brief: ��ȡ������Ƶ�߶�
		* @return: ������Ƶ�߶�
		*/
		int SourceHeight() const;

		/**
		* @brief: ��ȡ������Ƶ֡���ʱ��(����)
		* @return: ������Ƶ֡���ʱ��(����)
		*/
		unsigned char FrameSpan() const;

	private:
		//��Ƶ�������
		std::string _inputUrl;
		//������Ƶ
		AVFormatContext* _inputFormat;
		//��Ƶ��
		AVStream* _inputStream;
		//��Ƶ�����
		int _inputVideoIndex;
		//��ƵԴ���
		int _sourceWidth;
		//��ƵԴ�߶�
		int _sourceHeight;
		//������Ƶ֡���ʱ��(����)
		unsigned char _frameSpan;
	};
}


