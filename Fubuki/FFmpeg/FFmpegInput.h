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
	//��Ƶ����
	class FFmpegInput
	{
	public:
		/**
		* @brief: ���캯��
		*/
		FFmpegInput();

		/**
		* @brief: ��ʼ��ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* @brief: ж��ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* @brief: ��ʼ����Ƶ����
		* @param: inputUrl ��Ƶ�����ַ
		* @return: ��Ƶ״̬
		*/
		bool Init(const std::string& inputUrl);

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


