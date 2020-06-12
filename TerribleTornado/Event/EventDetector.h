#pragma once
#include "EventData.h"
#include "TrafficDetector.h"
#include "ImageConvert.h"

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
		EventDetector(int width, int height,MqttChannel* mqtt, bool debug);

		/**
		* @brief: 更新通道
		* @param: channel 通道
		*/
		void UpdateChannel(const EventChannel& channel);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer,int frameIndex,int frameSpan);

	private:

		//事件检测缓存
		class EventDetectCache
		{
		public:
			EventDetectCache()
				:FirstTimeStamp(0),LastTimeStamp(0), StartPark(false), StopPark(false),StartParkImage(), RetrogradePoints()
			{

			}
			//检测项第一次出现的时间戳，停车检测
			long long FirstTimeStamp;
			//检测项最后一次出现的时间戳，删除缓存，行人检测，停车检测
			long long LastTimeStamp;
			//是否已经开始计算停车，停车检测
			bool StartPark;
			//是否已经结束计算停车，停车检测
			bool StopPark;
			//开始停车的图片，停车检测
			std::string StartParkImage;
			//在区域逆行的检测点集合，逆行检测
			std::vector<Point> RetrogradePoints;
		};

		//事件车道缓存
		class EventLaneCache
		{
		public:
			EventLaneCache()
				:LaneIndex(0), LaneType(EventLaneType::None), Region(), XTrend(true), YTrend(true)
				, Congestion(false),LastReportTimeStamp(0), Items()
			{

			}

			//车道序号
			int LaneIndex;
			//车道类型
			EventLaneType LaneType;
			//当前检测区域
			Polygon Region;
			//x移动的趋势，true表示正向，逆行检测
			bool XTrend;
			//y移动的趋势，true表示正向，逆行检测
			bool YTrend;
			//判断的坐标系
			bool BaseAsX;

			//当前车道是否处于拥堵状态
			bool Congestion;
			//上一次上报的时间戳，拥堵检测
			long long LastReportTimeStamp;

			//车道内检测项集合
			std::map<std::string, EventDetectCache> Items;

		};

		/**
		* @brief: 绘制逆行事件图片
		* @param: jpgBase64 用于写入的字符串
		* @param: iveBuffer ive字节流
		* @param: points 检测点集合
		* @param: frameIndex 帧序号
		*/
		void DrawRetrograde(std::string* jpgBase64, const unsigned char* iveBuffer, const std::vector<Point>& points, int frameIndex);
		
		/**
		* @brief: 绘制逆行事件图片
		* @param: jpgBase64 用于写入的字符串
		* @param: iveBuffer ive字节流
		* @param: point 行人点
		* @param: frameIndex 帧序号
		*/
		void DrawPedestrain(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, int frameIndex);
		
		/**
		* @brief: 绘制逆行事件图片
		* @param: jpgBase64 用于写入的字符串
		* @param: iveBuffer ive字节流
		* @param: point 停车点
		* @param: frameIndex 帧序号
		*/
		void DrawPark(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, int frameIndex);
		
		/**
		* @brief: 绘制拥堵区域
		* @param: jpgBase64 用于写入的字符串
		* @param: iveBuffer ive字节流
		* @param: frameIndex 帧序号
		*/
		void DrawCongestion(std::string* jpgBase64, const unsigned char* iveBuffer, int frameIndex);

		/**
		* @brief: 绘制检测区域
		* @param: detectItems 检测项集合
		* @param: iveBuffer ive字节流
		* @param: frameIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex);

		//IO mqtt主题
		static const std::string EventTopic;
		//数据移除的时间间隔(毫秒)
		static const int DeleteSpan;
		//停车开始计算的时间间隔(毫秒)
		static const int ParkStartSpan;
		//停车结束计算的时间间隔(毫秒)
		static const int ParkEndSpan;
		//用于判断拥堵的车数量
		static const int CarCount;
		//上报拥堵事件间隔(毫秒)
		static const int ReportSpan;
		//逆行检测用到的移动像素
		static const double MovePixel;
		//判断逆行事件的点的数量
		static const int PointCount;

		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<EventLaneCache> _lanes;

	};

}

