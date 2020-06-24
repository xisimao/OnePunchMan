#pragma once
#include "DecodeChannel.h"
#include "HisiEncodeChannel.h"
#include "IVE_8UC3Handler.h"

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
			:TaskId(0),IveBuffer(NULL),FrameIndex(0), FrameSpan(0),Finished(false)
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
	//�����߳�
	class HisiDecodeChannel :public DecodeChannel
	{
	public:
		/**
		* @brief: ���캯��
		* @param: channelIndex ͨ�����
		*/
		HisiDecodeChannel(int channelIndex,HisiEncodeChannel* encodeChannel);
		
		/**
		* @brief: ��������
		*/
		~HisiDecodeChannel();

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
		* @return: ive֡����
		*/
		FrameItem GetTempIve();

		/**
		* @brief: ����һ֡��Ƶд�뵽bmp
		*/
		void WriteBmp();

	protected:
		ChannelStatus InitDecoder(const std::string& inputUrl);

		void UninitDecoder();

		DecodeResult Decode(const AVPacket* packet, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

	private:
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

		//�Ƿ���Ҫд��bmp�ļ�
		bool _writeBmp;

		//��ǰ��Ƶ��ȡ�Ƿ����
		bool _finished;
		//��ǰyuv���ݵ������
		unsigned char _taskId;
		//��ǰyuv���ݵ�֡���
		unsigned int _frameIndex;
		//��ǰyuv���ݵ�֡���ʱ��
		unsigned char _frameSpan;

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

		IVE_8UC3Handler _iveHandler;
		HisiEncodeChannel* _encodeChannel;
	};

}

