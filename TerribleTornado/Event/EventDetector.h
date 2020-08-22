#pragma once
#include "EventData.h"
#include "TrafficDetector.h"
#include "EncodeChannel.h"

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
		* @param: encodeChannel 编码通道
		*/
		EventDetector(int width, int height,MqttChannel* mqtt, EncodeChannel* encodeChannel);
		
		/**
		* @brief: 析构函数
		*/
		~EventDetector();

		/**
		* @brief: 更新通道
		* @param: taskId 任务号
		* @param: channel 通道
		*/
		void UpdateChannel(const unsigned char taskId, const EventChannel& channel);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, unsigned char taskId, const unsigned char* iveBuffer,unsigned int frameIndex, unsigned char frameSpan);

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
				:LaneIndex(0), LaneType(EventLaneType::None), Region(), XTrend(true), YTrend(true), BaseAsX(false)
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

		//事件编码缓存
		class EventEncodeCache
		{
		public:
			EventEncodeCache()
				:Json(), EncodeIndex(-1), FilePath()
			{

			}
			//事件json数据
			std::string Json;
			//编码器序号
			int EncodeIndex;
			//输出mp4文件地址
			std::string FilePath;
		};

		/**
		* @brief: 绘制车辆事件图片
		* @param: jpgBase64 用于写入的字符串
		* @param: iveBuffer ive字节流
		* @param: laneRegion 检测区域
		* @param: detectRegion 检测项区域
		* @param: frameIndex 帧序号
		*/
		void DrawRegion(std::string* jpgBase64,const unsigned char* iveBuffer, const Polygon& laneRegion, const Rectangle& detectRegion, unsigned int frameIndex);
	
		/**
		* @brief: 绘制拥堵区域
		* @param: jpgBase64 用于写入的字符串
		* @param: iveBuffer ive字节流
		* @param: laneRegion 检测区域
		* @param: frameIndex 帧序号
		*/
		void DrawRegion(std::string* jpgBase64, const unsigned char* iveBuffer, const Polygon& laneRegion, unsigned int frameIndex);

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
		//视频编码帧数
		static const int EncodeFrameCount;

		//任务编号
		int _taskId;

		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<EventLaneCache> _lanes;
		//等待编码的集合
		std::vector<EventEncodeCache> _encodes;
		//bgr字节流
		unsigned char* _bgrBuffer;
		//jpg字节流
		unsigned char* _jpgBuffer;
		//编码线程
		EncodeChannel* _encodeChannel;
		//视频文件临时存放字节流长度
		int _videoSize;
		//视频文件临时存放字节流
		unsigned char* _videoBuffer;
		
	};

}

