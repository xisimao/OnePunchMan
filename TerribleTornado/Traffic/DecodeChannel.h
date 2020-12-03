#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "FFmpegOutput.h"
#include "EncodeChannel.h"
#include "BGR24Handler.h"
#include "TrafficData.h"
#include "FrameHandler.h"

#ifndef _WIN32
#include "clientsdk.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#endif // !_WIN32

namespace OnePunchMan
{
	//������
	enum class DecodeResult
	{
		Handle,
		Skip,
		Error
	};

	//֡����
	class FrameItem
	{
	public:
		FrameItem()
			:TaskId(0), IveBuffer(NULL), FrameIndex(0), FrameSpan(0), Finished(false)
		{

		}
		//�����
		unsigned char TaskId;
		//ive�ֽ���
		unsigned char* IveBuffer;
		//֡���
		unsigned int FrameIndex;
		//֡��
		unsigned char FrameSpan;
		//��Ƶ�Ƿ��Ѿ���ȡ���
		bool Finished;
	};

	//��Ƶ֡��ȡ�߳�
	class DecodeChannel :public ThreadObject
	{
	public:
		/**
		* ���캯��
		* @param channelIndex ͨ�����
		* @param encodeChannel �����߳�
		* @param frameHandler ֡����
		*/
		DecodeChannel(int channelIndex, EncodeChannel* encodeChannel, FrameHandler* frameHandler);

		/**
		* ��ʼ��ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* ж��ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* ��ʼ��hisi sdk
		* @param videoCount ͨ������
		* @return ��ʼ���ɹ�����true,���򷵻�false
		*/
		static bool InitHisi(int videoCount);

		/**
		* ж��hisi sdk
		* @param videoCount ͨ������
		*/
		static void UninitHisi(int videoCount);

		/**
		* ��ʼ��������Ƶ
		* @param gbParameter �������
		*/
		static bool InitGB(const GbParameter& gbParameter);

		/**
		* ��ȡͨ����ַ
		* @param inputUrl ��ƵԴ��ַ
		* @param outputUrl rtmp�����ַ
		* @param channelType ͨ������
		* @param loop �Ƿ�ѭ������
		* @return ��ǰ������
		*/
		unsigned char UpdateChannel(const std::string& inputUrl, const std::string& outputUrl, ChannelType channelType, bool loop);

		/**
		* ���ͨ��
		*/
		void ClearChannel();


		/**
		* ��ȡ����ͨ����ַ
		* @return ����ͨ����ַ
		*/
		std::string InputUrl();

		/**
		* ��ȡͨ��״̬
		* @return ͨ��״̬
		*/
		ChannelStatus Status();

		/**
		* ��ȡ֡��
		* @return ֡��
		*/
		int FrameSpan();

		/**
		* ��ȡ����֡���
		* @return ����֡���
		*/
		int HandleSpan();

		/**
		* ��ȡ�����Ƶ������
		* @return ��Ƶ������
		*/
		int SourceWidth();

		/**
		* ��ȡ�����Ƶ����߶�
		* @return ��Ƶ����߶�
		*/
		int SourceHeight();

		//��������Ƶ���
		static const int DestinationWidth;
		//��������Ƶ�߶�
		static const int DestinationHeight;

	protected:
		void StartCore();

	private:
		/**
		* ��ʼ��������
		* @param playFd ��Ƶ���
		* @param frameType ֡����
		* @param buffer ֡�ֽ���
		* @param size ֡�ֽ�������
		* @param usr �û��Զ�������
		*/
		static void ReceivePacket(int playFd, int frameType, char* buffer, unsigned int size, void* usr);

		/**
		* ��ʼ����Ƶ����
		* @param inputUrl ��Ƶ�����ַ
		* @return ��Ƶ״̬
		*/
		bool InitInput(const std::string& inputUrl);

		/**
		* ������Ƶ����
		*/
		void UninitInput();

		/**
		* ��ʼ��������
		* @param inputUrl ����url
		* @return ��ʼ���ɹ�����true
		*/
		bool InitDecoder(const std::string& inputUrl);

		/**
		* ж�ؽ�����
		*/
		void UninitDecoder();

		/**
		* ����
		* @param packet ��Ƶ�ֽ���
		* @param size ��Ƶ�ֽ�������
		* @param taskId �����
		* @param frameIndex ��Ƶ֡���
		* @param frameSpan ��֡�ļ��ʱ��(����)
		* @return ����ɹ�����Handle,�Թ�����Ski,���򷵻�Error,����ʧ�ܻ�����߳�
		*/
		DecodeResult Decode(unsigned char* buffer, unsigned int size, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

		/**
		* ����,����windows����
		* @param packet ��Ƶ֡
		* @param taskId �����
		* @param frameIndex ��Ƶ֡���
		* @param frameSpan ��֡�ļ��ʱ��(����)
		* @return ����ɹ�����Handle,�Թ�����Ski,���򷵻�Error,����ʧ�ܻ�����߳�
		*/
		DecodeResult DecodeTest(const AVPacket* packet, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

		//����ʱ��(����)
		static const int ConnectSpan;
		//��������ʱ��(����)
		static const int GbSleepSpan;
		//��Ĵ���֡���,��������������ͻ��޸�ͨ��״̬
		static const int MaxHandleSpan;

		//�����½���
		static int GBLoginId;

		//���캯��ʱ�ı�
		//ͨ�����
		int _channelIndex;

		//����ͨ��ʱ�ı�
		//��Ƶ״̬ͬ����
		std::mutex _mutex;
		//��Ƶ�����ַ
		std::string _inputUrl;
		//��Ƶ�����ַ
		std::string _outputUrl;
		//ͨ������
		ChannelType _channelType;
		//��ǰ��Ƶ״̬
		ChannelStatus _channelStatus;
		//�����
		unsigned char _taskId;
		//�Ƿ�ѭ������
		bool _loop;

		//�߳��иı�
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
		//֡��
		unsigned int _frameIndex;
		//��һ�δ���֡�����
		unsigned int _lastframeIndex;
		//����֡�ļ��
		unsigned int _handleSpan;
		//��ǰ������Ƶ�������
		unsigned char _currentTaskId;
		//��ǰ���겥�ž��
		int _playHandler;

		//��Ƶ���
		FFmpegOutput _outputHandler;

		//debug�������
		AVCodecContext* _decodeContext;
		//������yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		//bgr�ֽ���
		uint8_t* _bgrBuffer;
		//yuvתbgr
		SwsContext* _bgrSwsContext;
		//bgrдbmp
		BGR24Handler _bgrHandler;

		//�첽����
#ifndef _WIN32
		bool _hasFrame;
		VIDEO_FRAME_INFO_S _tempFrame;
#endif // !_WIN32

		//����
		EncodeChannel* _encodeChannel;

		//����֡
		FrameHandler* _frameHandler;
	};

}

