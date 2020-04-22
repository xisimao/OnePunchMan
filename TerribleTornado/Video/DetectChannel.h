#pragma once
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "RecognChannel.h"
#include "ChannelDetector.h"
#include "YUV420SPHandler.h"
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


namespace TerribleTornado
{
	//����߳�
	class DetectChannel :public Saitama::ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: channelIndex ��Ƶ���
		* @param: width ��Ƶ�������
		* @param: height ��Ƶ�����߶�
		* @param: recogn ��Ƶ�߳�
		* @param: detector ͨ�����
		*/
		DetectChannel(int channelIndex,int width, int height,RecognChannel* recogn, ChannelDetector* detector);
		
		/**
		* @brief: ��������
		*/
		~DetectChannel();

		/**
		* @brief: ��ǰ����߳��Ƿ�δ��ʼ�������ڴ�������
		* @return: ���δ��ʼ�����ߵ�ǰ�߳����ڽ��м�ⷵ��true�����򷵻�false����ʾ���Խ����µ�yuv����
		*/
		bool IsBusy();

		/**
		* @brief: ����yuv����
		* @param: yuv yuv�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: packetIndex ��Ƶ֡���
		*/
		void HandleYUV(unsigned char* yuv, int width, int height, int packetIndex);

	protected:
		void StartCore();

	private:

		//��Ƶ֡����
		class FrameItem
		{
		public:
			//yuv420sp
			unsigned long long Yuv_phy_addr;
			uint8_t* YuvBuffer;
			uint8_t* YuvTempBuffer;
			int YuvSize;

			//bgr
			unsigned long long Bgr_phy_addr;
			uint8_t* BgrBuffer;
			int BgrSize;
			
			//��Ƶ֡���
			int PacketIndex;
			//��ǰ�������Ƿ��Ѿ�����yuv����
			bool HasValue;
		};

		//��ѯ��˯��ʱ��(����)
		static const int SleepTime;

		/**
		* @brief: yuvת8uc3
		* @return: ת���ɹ�����true�����򷵻�false
		*/
		bool YuvToBgr();

		//�߳��Ƿ��ʼ�����
		bool _inited;
		//ͨ�����
		int _channelIndex;
		//ͼƬ���
		int _width;
		//ͼƬ�߶�
		int _height;
		//����߳�
		RecognChannel* _recogn;
		//ͨ�����
		ChannelDetector* _detector;

		//��Ƶ֡����
		FrameItem _item;

		//detect
		std::vector<uint8_t*> _bgrs;
		std::vector<int> _indexes;
		std::vector<uint64_t> _timeStamps;
		std::vector<uint32_t> _widths;
		std::vector<uint32_t> _heights;
		std::vector<const char*> _params;
		std::vector<char> _result;

		//debug
		Fubuki::YUV420SPHandler* _yuvHandler;
		Fubuki::IVE_8UC3Handler* _bgrHandler;
	};

}

