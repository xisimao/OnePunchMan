#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "H264Handler.h"
#include "YUV420PHandler.h"
#include "BGR24Handler.h"
#include "MqttChannel.h"

#include "opencv2/opencv.hpp"

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
		//�ѽ���
		End,
		//����
		Normal,
		//�����ʼ������
		InputError,
		//�����ʼ������
		OutputError,
		//��������ʼ������
		DecoderError,
		//��ȡ��Ƶ֡����
		ReadError,
		//�������
		DecodeError
	};

	//��Ƶ֡��ȡ�߳�
	class FFmpegChannel :public ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: inputUrl ����url
		* @param: outputUrl ���url
		* @param: loop �Ƿ�ѭ������
		*/
		FFmpegChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop);

		/**
		* @brief: ��������
		*/
		virtual ~FFmpegChannel() {}

		/**
		* @brief: ��ʼ��ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* @brief: ж��ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* @brief: ��ȡͨ��״̬
		* @return: ͨ��״̬
		*/
		ChannelStatus Status();

	protected:
		void StartCore();

		/**
		* @brief: ��ʼ��������
		* @return: ��ʼ���ɹ�����ture�����򷵻�false����ʼ��ʧ�ܻ�����߳�
		*/
		virtual bool InitDecoder();
		
		/**
		* @brief: ж�ؽ�����
		*/
		virtual void UninitDecoder();

		/**
		* @brief: ����
		* @param: packet ��Ƶ֡
		* @param: packetIndex ��Ƶ֡���
		* @return: ����ɹ�����true�����򷵻�false������ʧ�ܻ�����߳�
		*/
		virtual bool Decode(const AVPacket* packet, int packetIndex);

		//��Ƶ�������
		std::string _inputUrl;
		AVFormatContext* _inputFormat;
		AVStream* _inputStream;
		int _inputVideoIndex;
		//��Ƶ������
		std::string _outputUrl;
		AVFormatContext* _outputFormat;
		AVStream* _outputStream;
		AVCodecContext* _outputCodec;

	private:
		/**
		* @brief: ��ʼ����Ƶ��ȡ
		* @return: ��Ƶ״̬
		*/
		ChannelStatus Init();

		/**
		* @brief: ж����Ƶ��ȡ
		*/
		void Uninit();

		//������Ƶ��ʼ������
		AVDictionary* _options;
		//�Ƿ�ѭ����ȡ
		bool _loop;
		//��ǰ��Ƶ״̬
		ChannelStatus _channelStatus;

		//debug
		//�������
		AVCodecContext* _decodeContext;
		//������yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		//bgr���
		int _bgrWidth;
		//bgr�߶�
		int _bgrHeight;
		//bgr�ֽ���
		uint8_t* _bgrBuffer;
		//yuvתbgr
		SwsContext* _bgrSwsContext;
		//h264д��
		H264Handler* _h264Handler;
		//yuv420pд��
		YUV420PHandler* _yuvHandler;
		//bgr24д��
		BGR24Handler* _bgrHandler;
	};

}

