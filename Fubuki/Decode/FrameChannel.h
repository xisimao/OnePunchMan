#pragma once
#include <string>
#include <bitset>

#include "Thread.h"

extern "C"
{
#include "libavformat/avformat.h"
}

namespace Fubuki
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
	class FrameChannel :public Saitama::ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: inputUrl ����url
		* @param: outputUrl ���url
		* @param: loop �Ƿ�ѭ������
		*/
		FrameChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop);

		/**
		* @brief: ��������
		*/
		virtual ~FrameChannel() {}

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
		virtual bool InitDecoder() { return true; };
		
		/**
		* @brief: ж�ؽ�����
		*/
		virtual void UninitDecoder() {};

		/**
		* @brief: ����
		* @param: packet ��Ƶ֡
		* @param: packetIndex ��Ƶ֡���
		* @return: ����ɹ�����true�����򷵻�false������ʧ�ܻ�����߳�
		*/
		virtual bool Decode(const AVPacket* packet, int packetIndex) { return true; }

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
	};

}

