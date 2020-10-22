#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "FFmpegOutput.h"
#include "EncodeChannel.h"
#include "BGR24Handler.h"
#include "TrafficData.h"

#ifndef _WIN32
#include "clientsdk.h"
#include "hi_common.h"
#include "hi_buffer.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vpss.h"
#include "hi_comm_avs.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "hi_comm_hdmi.h"
#include "hi_mipi.h"
#include "hi_comm_hdr.h"
#include "hi_comm_vgs.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#include "mpi_avs.h"
#include "mpi_region.h"
#include "mpi_audio.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "hi_math.h"
#include "hi_sns_ctrl.h"
#include "mpi_hdmi.h"
#include "mpi_hdr.h"
#include "mpi_vgs.h"
#include "mpi_ive.h"
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
		* ��������
		*/
		~DecodeChannel();

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

		/**
		* ��ȡ��ʱ��ŵ�ive����
		* @return ive֡����
		*/
		FrameItem GetTempIve();

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

		/**
		* д����ʱive����
		* @param taskId ������
		* @param yuv yuv����
		* @param frameIndex ֡���
		* @param finished ��Ƶ�Ƿ��ȡ���
		* @return ����true��ʾ�ɹ�д��,����false��ʾ��ǰ�Ѿ���yuv����
		*/
		bool SetTempIve(unsigned char taskId, const unsigned char* yuv, unsigned int frameIndex, unsigned char frameSpan, bool finished);

		/**
		* yuvתive
		* @return ת���ɹ�����true,���򷵻�false
		*/
		bool YuvToIve();

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
		//bgrдbmp
		BGR24Handler _bgrHandler;

		//�첽����
		//��ǰ��Ƶ��ȡ�Ƿ����
		bool _finished;
		//��ǰyuv���ݵ������
		unsigned char _tempTaskId;
		//��ǰyuv���ݵ�֡���
		unsigned int _tempFrameIndex;
		//��ǰyuv���ݵ�֡���ʱ��
		unsigned char _tempFrameSpan;

		//�������
		//yuv420sp
		int _yuvSize;
		//��ǰyuv�ֽ����Ƿ��Ѿ�����yuv����
		bool _yuvHasValue;
		//yuv����ʱ�ֽ���
		unsigned long long _yuv_phy_addr;
		uint8_t* _yuvBuffer;
		//ive����ʱ�ֽ���
		int _iveSize;
		unsigned long long _ive_phy_addr;
		uint8_t* _iveBuffer;

		//����
		EncodeChannel* _encodeChannel;
	};

}

