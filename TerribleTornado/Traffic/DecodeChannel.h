#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "FFmpegOutput.h"
#include "EncodeChannel.h"
#include "ImageConvert.h"
#include "TrafficData.h"

#ifndef _WIN32
#include "clientsdk.h"
#include "hi_comm_video.h"
#include "mpi_sys.h"
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
		* @param loginId �����½id
		* @param encodeChannel �����߳�
		*/
		DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel);

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
		* ������Ƶͨ��
		* @param channel ��Ƶͨ��
		* @return ��ǰ������
		*/
		unsigned char UpdateChannel(const TrafficChannel& channel);

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
		* ��ȡ�����Ƶ������
		* @return ��Ƶ������
		*/
		int SourceWidth();

		/**
		* ��ȡ�����Ƶ����߶�
		* @return ��Ƶ����߶�
		*/
		int SourceHeight();

		/**
		* ��ȡ��ʱ��ŵ�ive����
		* @return ive֡����
		*/
		FrameItem GetIve();

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
		* ����
		* @param packet ��Ƶ�ֽ���
		* @param size ��Ƶ�ֽ�������
		* @param taskId �����
		* @param frameIndex ��Ƶ֡���
		* @param frameSpan ��֡�ļ��ʱ��(����)
		* @return ����ɹ�����Handle,�Թ�����Ski,���򷵻�Error,����ʧ�ܻ�����߳�
		*/
		bool Decode(unsigned char* buffer, unsigned int size, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

#ifndef _WIN32
		/**
		* д����ʱive����
		* @param frame ��Ƶ֡,����NULL��ʾ��ɽ���
		* @return ����true��ʾ�ɹ�д��,����false��ʾ��ǰ�Ѿ���yuv����
		*/
		bool SetFrame(VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32


		//����ʱ��(����)
		static const int ConnectSpan;
		//��������ʱ��(����)
		static const int GbSleepSpan;
		//��Ĵ���֡���,��������������ͻ��޸�ͨ��״̬
		static const int MaxHandleSpan;

		//���캯��ʱ�ı�
		//ͨ�����
		int _channelIndex;
		//�����½id
		int _loginId;

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
		//�Ƿ�ͬ����⣬ȫ�ּ���ļ���Ƶʱִ��ͬ�����
		bool _syncDetect;
		
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
		//�ܹ���ȡ��֡��
		unsigned int _totalFrameCount;
		//�����֡��
		unsigned int _handleFrameCount;
		//��ǰ������Ƶ�������
		unsigned char _currentTaskId;
		//��ǰ���겥�ž��
		int _playHandler;

		//��Ƶ���
		FFmpegOutput _outputHandler;

		//�������
		//yuv�Ƿ�������
		bool _yuvHasData;
		//ͼ��ת��
		ImageConvert _image;

		//�첽����
		//��ǰ��Ƶ��ȡ�Ƿ����
		bool _finished;
		//��ǰyuv���ݵ������
		unsigned char _tempTaskId;
		//��ǰyuv���ݵ�֡���
		unsigned int _tempFrameIndex;
		//��ǰyuv���ݵ�֡���ʱ��
		unsigned char _tempFrameSpan;

		//����
		EncodeChannel* _encodeChannel;
	};

}

