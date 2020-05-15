#pragma once
#include "FFmpegChannel.h"
#include "DetectChannel.h"

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
	//解码线程
	class DecodeChannel :public FFmpegChannel
	{
	public:
		/**
		* @brief: 构造函数
		* @param: inputUrl 输入视频
		* @param: outputUrl 输出视频
		* @param: channelIndex 通道序号
		* @param: detectChannel 检测线程
		* @param: debug 是否处于调试模式,处于调试模式同步调用算法
		*/
		DecodeChannel(const std::string& inputUrl,const std::string& outputUrl,int channelIndex, DetectChannel* detectChannel, bool debug);
		
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

	protected:
		bool InitDecoder();

		void UninitDecoder();

		DecodeResult Decode(const AVPacket* packet, int frameIndex,int frameSpan);

	private:
		//通道序号
		int _channelIndex;

		//yuv字节流长度
		int _yuv420spSize;

		//检测线程
		DetectChannel* _detectChannel;
	};

}

