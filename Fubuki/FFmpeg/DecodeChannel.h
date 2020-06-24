#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "InputHandler.h"
#include "OutputHandler.h"
#include "BGR24Handler.h"

namespace OnePunchMan
{
	//������
	enum class DecodeResult
	{
		Handle,
		Skip,
		Error
	};

	//��Ƶ֡��ȡ�߳�
	class DecodeChannel :public ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: channelIndex ͨ�����
		*/
		DecodeChannel(int channelIndex);

		/**
		* @brief: ��������
		*/
		virtual ~DecodeChannel() {}

		/**
		* @brief: ��ʼ��ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* @brief: ж��ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* @brief: ��ȡͨ����ַ
		* @param: inputUrl ��ƵԴ��ַ
		* @param: outputUrl rtmp�����ַ		
		* @param: loop �Ƿ�ѭ������
		* @return: ��ǰ������
		*/
		unsigned char UpdateChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop);

		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

		/**
		* @brief: ��ȡ����ͨ����ַ
		* @return: ����ͨ����ַ
		*/
		std::string InputUrl();

		/**
		* @brief: ��ȡͨ��״̬
		* @return: ͨ��״̬
		*/
		ChannelStatus Status();

		/**
		* @brief: ��ȡ����֡���
		* @return: ����֡���
		*/
		int HandleSpan();

		/**
		* @brief: ��ȡ�����Ƶ������
		* @return: ��Ƶ������
		*/
		int SourceWidth();

		/**
		* @brief: ��ȡ�����Ƶ����߶�
		* @return: ��Ƶ����߶�
		*/
		int SourceHeight();

		//��������Ƶ���
		static const int DestinationWidth;
		//��������Ƶ�߶�
		static const int DestinationHeight;
	
	protected:
		void StartCore();

		/**
		* @brief: ��ʼ��������
		* @param: inputUrl ����url
		* @return: ��ʼ���ɹ�����true
		*/
		virtual ChannelStatus InitDecoder(const std::string& inputUrl);

		/**
		* @brief: ж�ؽ�����
		*/
		virtual void UninitDecoder();

		/**
		* @brief: ����
		* @param: packet ��Ƶ֡
		* @param: taskId �����
		* @param: frameIndex ��Ƶ֡���
		* @param: frameSpan ��֡�ļ��ʱ��(����)
		* @return: ����ɹ�����Handle���Թ�����Ski�����򷵻�Error������ʧ�ܻ�����߳�
		*/
		virtual DecodeResult Decode(const AVPacket* packet, unsigned char taskId,unsigned int frameIndex,unsigned char frameSpan);

		//ͨ�����
		int _channelIndex;

	private:
		//����ʱ��(��)
		static const int ConnectSpan;

		//��Ƶ״̬ͬ����
		std::mutex _mutex;
		//��Ƶ�������
		std::string _inputUrl;
		InputHandler _inputHandler;

		//��Ƶ������
		std::string _outputUrl;
		OutputHandler _outputHandler;

		//�����
		unsigned char _taskId;
		//��ǰ��Ƶ״̬
		ChannelStatus _channelStatus;
		//�Ƿ�ѭ������
		bool _loop;

		//debug
		//�������
		AVCodecContext* _decodeContext;
		//������yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		//bgr�ֽ���
		uint8_t* _bgrBuffer;
		//yuvתbgr
		SwsContext* _bgrSwsContext;
		//��һ�δ���֡�����
		unsigned int _lastframeIndex;
		//����֡�ļ��
		unsigned int _handleSpan;
		//bgrдbmp
		BGR24Handler _bgrHandler;
	};

}

