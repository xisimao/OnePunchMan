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
	//检测线程
	class DetectChannel :public Saitama::ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: channelIndex 视频序号
		* @param: width 视频解码后宽度
		* @param: height 视频解码后高度
		* @param: recogn 视频线程
		* @param: detector 通道检测
		*/
		DetectChannel(int channelIndex,int width, int height,RecognChannel* recogn, ChannelDetector* detector);
		
		/**
		* @brief: 析构函数
		*/
		~DetectChannel();

		/**
		* @brief: 当前检测线程是否未初始化或正在处理数据
		* @return: 如果未初始化或者当前线程正在进行检测返回true，否则返回false，表示可以接收新的yuv数据
		*/
		bool IsBusy();

		/**
		* @brief: 处理yuv数据
		* @param: yuv yuv字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: packetIndex 视频帧序号
		*/
		void HandleYUV(unsigned char* yuv, int width, int height, int packetIndex);

	protected:
		void StartCore();

	private:

		//视频帧数据
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
			
			//视频帧序号
			int PacketIndex;
			//当前数据项是否已经有了yuv数据
			bool HasValue;
		};

		//轮询中睡眠时间(毫秒)
		static const int SleepTime;

		/**
		* @brief: yuv转8uc3
		* @return: 转换成功返回true，否则返回false
		*/
		bool YuvToBgr();

		//线程是否初始化完成
		bool _inited;
		//通道序号
		int _channelIndex;
		//图片宽度
		int _width;
		//图片高度
		int _height;
		//检测线程
		RecognChannel* _recogn;
		//通道检测
		ChannelDetector* _detector;

		//视频帧数据
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

