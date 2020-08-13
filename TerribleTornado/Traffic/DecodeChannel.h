#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "FFmpegInput.h"
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
		* @brief: ���캯��
		* @param: channelIndex ͨ�����
		* @param: loginId �����½id
		* @param: encodeChannel �����߳�
		*/
		DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel);

		/**
		* @brief: ��������
		*/
		~DecodeChannel();

		/**
		* @brief: ��ʼ��hisi sdk
		* @param: videoCount ͨ������
		* @return: ��ʼ���ɹ�����true�����򷵻�false
		*/
		static bool InitHisi(int videoCount);

		/**
		* @brief: ж��hisi sdk
		* @param: videoCount ͨ������
		*/
		static void UninitHisi(int videoCount);

		/**
		* @brief: ��ȡͨ����ַ
		* @param: inputUrl ��ƵԴ��ַ
		* @param: outputUrl rtmp�����ַ		
		* @param: channelType ͨ������		
		* @param: loop �Ƿ�ѭ������
		* @return: ��ǰ������
		*/
		unsigned char UpdateChannel(const std::string& inputUrl, const std::string& outputUrl, ChannelType channelType, bool loop);

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
		* @brief: ��ȡ֡��
		* @return: ֡��
		*/
		int FrameSpan();

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

		/**
		* @brief: ��ȡ��ʱ��ŵ�ive����
		* @return: ive֡����
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
		* @brief: ��ʼ��������
		* @param: playFd ��Ƶ���
		* @param: frameType ֡����
		* @param: buffer ֡�ֽ���
		* @param: size ֡�ֽ�������
		* @param: usr �û��Զ�������
		*/
		static void ReceivePacket(int playFd, int frameType, char* buffer, unsigned int size, void* usr);

		/**
		* @brief: ��ʼ��������
		* @param: inputUrl ����url
		* @return: ��ʼ���ɹ�����true
		*/
		ChannelStatus InitDecoder(const std::string& inputUrl);

		/**
		* @brief: ж�ؽ�����
		*/
		void UninitDecoder();

		/**
		* @brief: ����
		* @param: packet ��Ƶ֡
		* @param: taskId �����
		* @param: frameIndex ��Ƶ֡���
		* @param: frameSpan ��֡�ļ��ʱ��(����)
		* @return: ����ɹ�����Handle���Թ�����Ski�����򷵻�Error������ʧ�ܻ�����߳�
		*/
		DecodeResult Decode(const AVPacket* packet, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

		/**
		* @brief: д����ʱive����
		* @param: taskId ������
		* @param: yuv yuv����
		* @param: frameIndex ֡���
		* @param: finished ��Ƶ�Ƿ��ȡ���
		* @return: ����true��ʾ�ɹ�д�룬����false��ʾ��ǰ�Ѿ���yuv����
		*/
		bool SetTempIve(unsigned char taskId, const unsigned char* yuv, unsigned int frameIndex, unsigned char frameSpan, bool finished);

		/**
		* @brief: yuvתive
		* @return: ת���ɹ�����true�����򷵻�false
		*/
		bool YuvToIve();

		//����ʱ��(����)
		static const int ConnectSpan;
		//��������ʱ��(����)
		static const int GbSleepSpan;
		//��Ĵ���֡�������������������ͻ��޸�ͨ��״̬
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
		//��Ƶ����
		FFmpegInput _inputHandler;
		//��Ƶ���
		FFmpegOutput _outputHandler;
		//֡���ʱ��(����)
		unsigned char _frameSpan;
		//֡��
		unsigned int _frameIndex;
		//��һ�δ���֡�����
		unsigned int _lastframeIndex;
		//����֡�ļ��
		unsigned int _handleSpan;
		//֡ʱ��
		long long _duration;
		//��ǰ������Ƶ�������
		unsigned char _currentTaskId;
		//��ǰ���겥�ž��
		int _playHandler;

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

		//����ʹ�õ�packet
		AVPacket* _gbPacket;

		//����
		EncodeChannel* _encodeChannel;
		//�Ƿ��Ѿ�����i֡
		bool _gotIFrame;

	};

}

