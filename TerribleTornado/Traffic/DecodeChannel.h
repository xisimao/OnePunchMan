#pragma once
#include "FFmpegChannel.h"
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
	//帧数据
	class FrameItem
	{
	public:
		FrameItem()
			:IveBuffer(NULL),FrameIndex(0), FrameSpan(0), Finished(false)
		{

		}
		//ive字节流
		unsigned char* IveBuffer;
		//帧序号
		int FrameIndex;
		//帧率
		int FrameSpan;
		//视频是否已经读取完成
		bool Finished;
	};
	//解码线程
	class DecodeChannel :public FFmpegChannel
	{
	public:
		/**
		* @brief: 构造函数
		* @param: channelIndex 通道序号
		*/
		DecodeChannel(int channelIndex);
		
		/**
		* @brief: 析构函数
		*/
		~DecodeChannel();

		/**
		* @brief: 初始化hisi sdk
		* @param: videoCount 通道总数
		* @return: 初始化成功返回true，否则返回false
		*/
		static bool InitHisi(int videoCount);

		/**
		* @brief: 卸载hisi sdk
		* @param: videoCount 通道总数
		*/
		static void UninitHisi(int videoCount);

		/**
		* @brief: 获取临时存放的ive数据
		* @return: ive帧数据
		*/
		FrameItem GetTempIve();

	protected:
		ChannelStatus InitDecoder(const std::string& inputUrl);

		void UninitDecoder();

		DecodeResult Decode(const AVPacket* packet, int frameIndex,int frameSpan);

	private:
		/**
		* @brief: 写入临时ive数据
		* @param: yuv yuv数据
		* @param: frameIndex 帧序号
		* @param: finished 视频是否读取完成
		* @return: 返回true表示成功写入，返回false表示当前已经有yuv数据
		*/
		bool SetTempIve(const unsigned char* yuv, int frameIndex,bool finished);

		/**
		* @brief: yuv转ive
		* @return: 转换成功返回true，否则返回false
		*/
		bool YuvToIve();

		//当前写入yuv时的帧序号
		int _frameIndex;
		//视频是否读取完成
		bool _finished;

		//yuv420sp
		int _yuvSize;
		//当前yuv字节流是否已经有了yuv数据
		bool _yuvHasValue;
		//yuv操作时字节流
		unsigned long long _yuv_phy_addr;
		uint8_t* _yuvBuffer;

		//ive操作时字节流
		int _iveSize;
		unsigned long long _ive_phy_addr;
		uint8_t* _iveBuffer;

		//ive写bmp
		IVE_8UC3Handler _iveHandler;
	};

}

