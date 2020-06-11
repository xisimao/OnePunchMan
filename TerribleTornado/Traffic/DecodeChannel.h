#pragma once
#include "FFmpegChannel.h"
#include "IVE_8UC3Handler.h"
#include "YUV420SPHandler.h"

extern "C"
{
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#ifndef _WIN32
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
	//֡����
	class FrameItem
	{
	public:
		FrameItem()
			:IveBuffer(NULL), YuvBuffer(NULL),FrameIndex(0), FrameSpan(0)
		{

		}
		//ive�ֽ���
		unsigned char* IveBuffer;
		//yuv�ֽ���
		unsigned char* YuvBuffer;
		//֡���
		int FrameIndex;
		//֡��
		int FrameSpan;

	};
	//�����߳�
	//���:yuv420sp�ֽ���,ive�ֽ���
	class DecodeChannel :public FFmpegChannel
	{
	public:
		/**
		* @brief: ���캯��
		* @param: channelIndex ͨ�����
		* @param: debug �Ƿ��ڵ���ģʽ,���ڵ���ģʽͬ�������㷨
		*/
		DecodeChannel(int channelIndex, bool debug);
		
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
		* @brief: ��ȡ��ʱ��ŵ�ive����
		* @param: needYuv �Ƿ���Ҫ��Ӧ��yuv����
		* @return: ive֡����
		*/
		FrameItem GetTempIve(bool needYuv);

		/**
		* @brief: ����һ֡��Ƶд�뵽bmp
		*/
		void WriteBmp();

	protected:
		ChannelStatus InitDecoder(const std::string& inputUrl);

		void UninitDecoder();

		DecodeResult Decode(const AVPacket* packet, int frameIndex,int frameSpan);

	private:
		/**
		* @brief: д����ʱive����
		* @param: yuv yuv����
		* @param: frameIndex ֡���
		* @return: ����true��ʾ�ɹ�д�룬����false��ʾ��ǰ�Ѿ���yuv����
		*/
		bool SetTempIve(const unsigned char* yuv, int frameIndex);

		/**
		* @brief: yuvתive
		* @return: ת���ɹ�����true�����򷵻�false
		*/
		bool YuvToIve();

		//ͨ�����
		int _channelIndex;
		//�Ƿ���Ҫд��bmp�ļ�
		bool _writeBmp;
		//��ǰд��yuvʱ��֡���
		int _frameIndex;

		//yuv420sp
		int _yuvSize;
		//��ǰyuv�ֽ����Ƿ��Ѿ�����yuv����
		bool _yuvHasValue;
		//yuv����ʱ�ֽ���
		unsigned long long _yuv_phy_addr;
		uint8_t* _yuvBuffer;
		//yuv��ʱ�ֽ���
		uint8_t* _tempYuvBuffer;

		//ive����ʱ�ֽ���
		int _iveSize;
		unsigned long long _ive_phy_addr;
		uint8_t* _iveBuffer;

		//iveдbmp
		IVE_8UC3Handler _iveHandler;
		YUV420SPHandler _yuvHandler;
	};

}

