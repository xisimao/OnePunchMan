#pragma once
#include "FFmpegChannel.h"
#include "YUV420SPHandler.h"
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
		* @param: loop 是否循环
		* @param: channelIndex 通道序号
		* @param: detectChannel 检测线程
		*/
		DecodeChannel(const std::string& inputUrl,const std::string& outputUrl,bool loop,int channelIndex, DetectChannel* detectChannel);
		
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
		* @brief: 获取输入通道地址
		* @return: 输入通道地址
		*/
		std::string& InputUrl();

		//解码后的视频宽度
		static const int VideoWidth;
		//解码后的视频高度
		static const int VideoHeight;

	protected:
		int InitCore();

		void UninitCore();

		bool Decode(const AVPacket* packet, int packetIndex);

	private:

		/**
		* @brief: 使用hisi解码
		* @param: packet 视频帧
		* @param: packetIndex 视频帧序号
		* @return: 解码成功返回true，否则返回false
		*/
		bool DecodeByHisi(const AVPacket* packet, int packetIndex);
		
		/**
		* @brief: 使用ffmpeg解码
		* @param: packet 视频帧
		* @param: packetIndex 视频帧序号
		* @return: 解码成功返回true，否则返回false
		*/
		bool DecodeByFFmpeg(const AVPacket* packet, int packetIndex);

		//通道序号
		int _channelIndex;

		//是否使用ffmpeg解码，优先使用hisi
		bool _useFFmpeg;

		//ffmpeg解码相关
		AVCodecContext* _decodeContext;
		//解码后的yuv数据
		AVFrame* _yuvFrame;
		//yuv420sp数据字节流
		uint8_t* _yuv420spBuffer;
		//yuv420sp数据字节流长度
		int _yuv420spSize;
		//yuv420sp帧数据
		AVFrame* _yuv420spFrame;
		//yuv420sp转换
		SwsContext* _yuv420spSwsContext;
		//检测线程
		DetectChannel* _detectChannel;
	
	};

}

