#pragma once
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "RecognChannel.h"
#include "TrafficDetector.h"
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
	//����߳�
	class DetectChannel :public ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: detectIndex ������
		* @param: channelCount ����ͨ���ĸ���
		* @param: width ��Ƶ�������
		* @param: height ��Ƶ�����߶�
		* @param: recogn ��Ƶ�߳�
		* @param: detector ͨ�����
		*/
		DetectChannel(int detectIndex,int channelCount,int width, int height,RecognChannel* recogn, const std::vector<TrafficDetector*>& detectors);
	
		/**
		* @brief: ��������
		*/
		~DetectChannel();

		/**
		* @brief: ��ǰ����߳��Ƿ�δ��ʼ��
		* @return: ���δ��ʼ�����ߵ�ǰ�߳����ڽ��м�ⷵ��true�����򷵻�false
		*/
		bool Inited();

		/**
		* @brief: ����һ֡д�뵽bmp�ļ�
		*/
		void WriteBmp(int channelIndex);

		/**
		* @brief: ��ǰ����߳��Ƿ�δ��ʼ�������ڴ�������
		* @param: channelIndex ͨ�����
		* @return: ���δ��ʼ�����ߵ�ǰ�߳����ڽ��м�ⷵ��true�����򷵻�false����ʾ���Խ����µ�yuv����
		*/
		bool IsBusy(int channelIndex);

		/**
		* @brief: ����yuv����
		* @param: channelIndex ͨ�����
		* @param: yuv yuv�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: frameIndex ��Ƶ֡���
		* @param: frameSpan ��Ƶ֡���ʱ��(����)
		*/
		void HandleYUV(int channelIndex,const unsigned char* yuv, int width, int height, int frameIndex,int frameSpan);

	protected:
		void StartCore();

	private:

		//��Ƶ֡����
		class FrameItem
		{
		public:
			FrameItem()
				: ChannelIndex(0),YuvSize(0),YuvTempBuffer(NULL), TempHasValue(false),Yuv_phy_addr(0),YuvBuffer(NULL)
				, IveSize(0), Ive_phy_addr(0), IveBuffer(NULL)
				, FrameTempIndex(0),FrameIndex(0),FrameSpan(0), Param(), WriteBmp(false)
			{

			}
			//ͨ�����
			int ChannelIndex;
			//yuv420sp
			int YuvSize;
			//yuv�ֽ�����ʱ���
			uint8_t* YuvTempBuffer;
			//��ǰyuv��ʱ�ֽ����Ƿ��Ѿ�����yuv����
			bool TempHasValue;
			//yuv����ʱ�ֽ���
			unsigned long long Yuv_phy_addr;
			uint8_t* YuvBuffer;

			//ive����ʱ�ֽ���
			int IveSize;
			unsigned long long Ive_phy_addr;
			uint8_t* IveBuffer;

			//��Ƶ֡�����ʱ���
			int FrameTempIndex;
			//��Ƶ֡���
			int FrameIndex;
			//��Ƶ֡���ʱ��(����)
			int FrameSpan;

			//��������
			std::string Param;
	
			//�Ƿ���Ҫд��ͼƬ
			bool WriteBmp;
		};

	private:
		/**
		* @brief: ��json�����л�ȡ������
		* @param: items ������
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: ��json�����л�ȡʶ�����
		* @param: items ʶ�����
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		* @param: channelIndex ͨ�����
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key,int channelIndex);

		/**
		* @brief: yuvתive
		* @param: frameItem ��Ƶ֡����
		* @return: ת���ɹ�����true�����򷵻�false
		*/
		bool YuvToIve(FrameItem* frameItem);


		int GetFrameItemIndex(int channelIndex);

		//��ѯ��˯��ʱ��(����)
		static const int SleepTime;
		//�������
		static const std::string DetectTopic;

		//�߳��Ƿ��ʼ�����
		bool _inited;
		//������
		int _detectIndex;
		//����ͨ������
		int _channelCount;
		//ͼƬ���
		int _width;
		//ͼƬ�߶�
		int _height;
		//����߳�
		RecognChannel* _recogn;
		//ͨ����⼯��
		std::vector<TrafficDetector*> _detectors;
		//��Ƶ֡����
		std::vector<FrameItem> _frameItems;
		//iveдbmp
		IVE_8UC3Handler _iveHandler;

		//detect
		std::vector<uint8_t*> _ives;
		std::vector<int> _indexes;
		std::vector<uint64_t> _timeStamps;
		std::vector<uint32_t> _widths;
		std::vector<uint32_t> _heights;
		std::vector<const char*> _params;
		std::vector<char> _result;

	};

}

