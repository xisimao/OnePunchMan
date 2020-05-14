#pragma once
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "RecognChannel.h"
#include "TrafficDetector.h"

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
	//检测线程
	class DetectChannel :public ThreadObject
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
		DetectChannel(int channelIndex,int width, int height,RecognChannel* recogn, TrafficDetector* detector);
		
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
		* @param: frameIndex 视频帧序号
		* @param: frameSpan 视频帧间隔时间(毫秒)
		*/
		void HandleYUV(unsigned char* yuv, int width, int height, int frameIndex,int frameSpan);

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
			unsigned long long Ive_phy_addr;
			uint8_t* IveBuffer;
			int IveSize;

			//视频帧序号
			int frameIndex;
			//视频帧间隔时间(毫秒)
			int frameSpan;
			//当前数据项是否已经有了yuv数据
			bool HasValue;
		};

	private:
		/**
		* @brief: 从json数据中获取检测项集合
		* @param: items 检测项集合
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: 从json数据中获取识别项集合
		* @param: items 识别项集合
		* @param: jd json解析
		* @param: key 检测类型，机动车，非机动和和行人
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: yuv转8uc3
		* @return: 转换成功返回true，否则返回false
		*/
		bool YuvToIve();

		//轮询中睡眠时间(毫秒)
		static const int SleepTime;

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
		TrafficDetector* _detector;

		//视频帧数据
		FrameItem _item;

		//detect
		std::vector<uint8_t*> _ives;
		std::vector<int> _indexes;
		std::vector<uint64_t> _timeStamps;
		std::vector<uint32_t> _widths;
		std::vector<uint32_t> _heights;
		std::vector<const char*> _params;
		std::vector<char> _result;
		std::string _param;
	};

}

