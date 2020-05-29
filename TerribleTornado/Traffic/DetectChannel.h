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
	//检测线程
	class DetectChannel :public ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: detectIndex 检测序号
		* @param: channelCount 处理通道的个数
		* @param: width 视频解码后宽度
		* @param: height 视频解码后高度
		* @param: recogn 视频线程
		* @param: detector 通道检测
		*/
		DetectChannel(int detectIndex,int channelCount,int width, int height,RecognChannel* recogn, const std::vector<TrafficDetector*>& detectors);
	
		/**
		* @brief: 析构函数
		*/
		~DetectChannel();

		/**
		* @brief: 当前检测线程是否未初始化
		* @return: 如果未初始化或者当前线程正在进行检测返回true，否则返回false
		*/
		bool Inited();

		/**
		* @brief: 将下一帧写入到bmp文件
		*/
		void WriteBmp(int channelIndex);

		/**
		* @brief: 当前检测线程是否未初始化或正在处理数据
		* @param: channelIndex 通道序号
		* @return: 如果未初始化或者当前线程正在进行检测返回true，否则返回false，表示可以接收新的yuv数据
		*/
		bool IsBusy(int channelIndex);

		/**
		* @brief: 处理yuv数据
		* @param: channelIndex 通道序号
		* @param: yuv yuv字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: frameIndex 视频帧序号
		* @param: frameSpan 视频帧间隔时间(毫秒)
		*/
		void HandleYUV(int channelIndex,const unsigned char* yuv, int width, int height, int frameIndex,int frameSpan);

	protected:
		void StartCore();

	private:

		//视频帧数据
		class FrameItem
		{
		public:
			FrameItem()
				: ChannelIndex(0),YuvSize(0),YuvTempBuffer(NULL), TempHasValue(false),Yuv_phy_addr(0),YuvBuffer(NULL)
				, IveSize(0), Ive_phy_addr(0), IveBuffer(NULL)
				, FrameTempIndex(0),FrameIndex(0),FrameSpan(0), Param(), WriteBmp(false)
			{

			}
			//通道序号
			int ChannelIndex;
			//yuv420sp
			int YuvSize;
			//yuv字节流临时存放
			uint8_t* YuvTempBuffer;
			//当前yuv临时字节流是否已经有了yuv数据
			bool TempHasValue;
			//yuv操作时字节流
			unsigned long long Yuv_phy_addr;
			uint8_t* YuvBuffer;

			//ive操作时字节流
			int IveSize;
			unsigned long long Ive_phy_addr;
			uint8_t* IveBuffer;

			//视频帧序号临时存放
			int FrameTempIndex;
			//视频帧序号
			int FrameIndex;
			//视频帧间隔时间(毫秒)
			int FrameSpan;

			//车道参数
			std::string Param;
	
			//是否需要写入图片
			bool WriteBmp;
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
		* @param: channelIndex 通道序号
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key,int channelIndex);

		/**
		* @brief: yuv转ive
		* @param: frameItem 视频帧数据
		* @return: 转换成功返回true，否则返回false
		*/
		bool YuvToIve(FrameItem* frameItem);


		int GetFrameItemIndex(int channelIndex);

		//轮询中睡眠时间(毫秒)
		static const int SleepTime;
		//检测主题
		static const std::string DetectTopic;

		//线程是否初始化完成
		bool _inited;
		//检测序号
		int _detectIndex;
		//处理通道个数
		int _channelCount;
		//图片宽度
		int _width;
		//图片高度
		int _height;
		//检测线程
		RecognChannel* _recogn;
		//通道检测集合
		std::vector<TrafficDetector*> _detectors;
		//视频帧数据
		std::vector<FrameItem> _frameItems;
		//ive写bmp
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

