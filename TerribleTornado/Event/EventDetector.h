#pragma once
#include "EventData.h"
#include "TrafficDetector.h"

namespace OnePunchMan
{
	//事件类型
	enum class EventType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Congestion = 3,
		Retrograde = 4
	};

	//通道检测
	class EventDetector :public TrafficDetector
	{
	public:
		/**
		* @brief: 构造函数
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: mqtt mqtt
		* @param: debug 是否处于调试模式，处于调试模式则输出画线后的bmp
		*/
		EventDetector(int width, int height, MqttChannel* mqtt, bool debug);

		/**
		* @brief: 更新通道
		* @param: channel 通道
		*/
		void UpdateChannel(const EventChannel& channel);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, long long packetIndex);

		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer, const std::string& recognJson);

	private:

		//事件检测缓存
		class EventDetectCache
		{
		public:
			EventDetectCache()
				:FirstTimeStamp(0),LastTimeStamp(0), StartPark(false), StopPark(false),StartParkImage(), StopRetrograde(false), HitPoint()
			{

			}
			//检测项第一次出现的时间戳，停车检测
			long long FirstTimeStamp;
			//检测项第二次出现的时间戳，删除缓存，行人检测，停车检测
			long long LastTimeStamp;
			//是否开始计算停车，停车检测
			bool StartPark;
			//是否开始计算停车，停车检测
			bool StopPark;
			//开始停车的图片，停车检测
			std::string StartParkImage;
			//停止检测逆行
			bool StopRetrograde;
			//检测点，逆行检测
			Point HitPoint;
		};

		//事件车道缓存
		class EventLaneCache
		{
		public:
			EventLaneCache()
				:LaneId(), LaneType(EventLaneType::None), Region(), Items(), LastReportTimeStamp(0), XTrend(true), YTrend(true)
			{

			}

			//车道编号
			std::string LaneId;
			//车道类型
			EventLaneType LaneType;
			//当前检测区域
			Polygon Region;
			//车道内检测项集合
			std::map<std::string, EventDetectCache> Items;
			//上一次上报的时间戳，拥堵检测
			long long LastReportTimeStamp;
			//x移动的趋势，true表示正向，逆行检测
			bool XTrend;
			//y移动的趋势，true表示正向，逆行检测
			bool YTrend;
		};

		/**
		* @brief: 绘制检测区域
		* @param: detectItems 检测项集合
		* @param: iveBuffer ive字节流
		* @param: packetIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex);
		
		/**
		* @brief: 获取当前帧的jpg base64字符串
		* @param: jpgBase64 用于写入的字符串
		* @param: iveBuffer ive字节流
		* @param: packetIndex 帧序号
		*/
		void GetJpgBase64(std::string* jpgBase64,const unsigned char* iveBuffer, long long packetIndex);

		//IO mqtt主题
		static const std::string EventTopic;
		//数据移除的时间间隔(毫秒)
		static const int DeleteSpan;
		//停车开始计算的时间间隔(毫秒)
		static const int ParkStartSpan;
		//停车结束计算的时间间隔(毫秒)
		static const int ParkEndSpan;
		//用于判断拥堵的车数量
		static const int CarsCount;
		//上报拥堵事件间隔(毫秒)
		static const int ReportSpan;

		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<EventLaneCache> _lanes;

	};

}

