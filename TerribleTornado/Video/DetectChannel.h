#pragma once
#include <vector>

#include "SeemmoSDK.h"
#include "FrameChannel.h"
#include "RecognChannel.h"
#include "JsonFormatter.h"
#include "LaneDetector.h"
#include "MqttChannel.h"

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

		bool HasValue;
		int CurrentPacketIndex;
		int LastPacketIndex;
		int PacketSpan;
	};

	class DetectChannel :public Saitama::ThreadObject
	{
	public:
		DetectChannel(int channelIndex,int width, int height, MqttChannel* mqtt, const std::vector<LaneDetector*>& lanes,RecognChannel* recogn);

		~DetectChannel();

		bool IsBusy(int index);

		void HandleYUV(unsigned char* yuv, int width, int height, int packetIndex,int index);

		static const int ItemCount;

	protected:
		void StartCore();

	private:
		//轮询中睡眠时间(毫秒)
		static const int SleepTime;

		//IO状态
		static const std::string IOTopic;
	


		static void GetRecognGuids(std::vector<std::string>* guids, const Saitama::JsonDeserialization& jd, const std::string& key1,const std::string& key2);

		bool YuvToBgr(FrameItem& item);

		/**
		* @brief: 从json数据中获取检测项集合
		* @param: json json数据
		* @param: key 检测项字段的键
		* @return: 检测项集合
		*/
		static void GetDetecItems(std::map<std::string, DetectItem>* items, const Saitama::JsonDeserialization& jd, const std::string& key);

		void HandleDetect(const std::map<std::string, DetectItem>& detectItems,long long timeStamp);
		
		int _detectIndex;
		int _width;
		int _height;
		MqttChannel* _mqtt;
		std::string _yuvTopic;
		std::mutex _laneMutex;
		std::vector<LaneDetector*> _lanes;

		RecognChannel* _recogn;

		std::mutex _frameMutex;

		std::vector<FrameItem> _items;

		//detect
		std::vector<uint8_t*> _bgrs;
		std::vector<uint8_t*> _yuvs;
		std::vector<int> _indexes;
		std::vector<uint64_t> _timeStamps;
		std::vector<uint32_t> _widths;
		std::vector<uint32_t> _heights;
		std::vector<const char*> _params;
		std::vector<char> _result;


	};

}

