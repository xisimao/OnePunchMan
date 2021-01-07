#pragma once
#include "TrafficData.h"
#include "ImageConvert.h"
#include "EncodeChannel.h"
#include "DataChannel.h"
#include "ImageDrawing.h"

namespace OnePunchMan
{
	//通道检测
	class EventDetector 
	{
	public:
		/**
		* 构造函数
		* @param width 图片宽度
		* @param height 图片高度
		* @param encodeChannel 编码线程
		* @param dataChannel 数据线程
		*/
		EventDetector(int width, int height, EncodeChannel* encodeChannel, DataChannel* dataChannel);
		
		/**
		* 析构函数
		*/
		~EventDetector();

		/**
		* 初始化事件参数配置
		* @param jd json配置
		*/
		static void Init(const JsonDeserialization& jd);

		/**
		* 更新通道
		* @param taskId 任务号
		* @param channel 通道
		*/
		void UpdateChannel(const unsigned char taskId, const TrafficChannel& channel);

		/**
		* 清空通道
		*/
		void ClearChannel();

		/**
		* 获取车道是否初始化成功
		* @return 车道是否初始化成功
		*/
		bool LanesInited() const;

		/**
		* 事件检测
		* @param items 检测数据项集合
		* @param timeStamp 时间戳
		* @param taskId 任务编号
		* @param iveBuffer 图片字节流
		* @param frameIndex 帧序号
		* @param frameSpan 帧间隔时间(毫秒)
		*/
		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, unsigned char taskId, const unsigned char* iveBuffer,unsigned int frameIndex, unsigned char frameSpan);

	private:
		//事件检测缓存
		class EventDetectCache
		{
		public:
			EventDetectCache()
				:FirstTimeStamp(0),LastTimeStamp(0), StartPark(false), StopPark(false),StartParkImage(), RetrogradePoints()
			{

			}
			//检测项第一次出现的时间戳,停车检测
			long long FirstTimeStamp;
			//检测项最后一次出现的时间戳,删除缓存,行人检测,停车检测
			long long LastTimeStamp;
			//是否已经开始计算停车,停车检测
			bool StartPark;
			//是否已经结束计算停车,停车检测
			bool StopPark;
			//开始停车的图片,停车检测
			std::string StartParkImage;
			//在区域逆行的检测点集合,逆行检测
			std::vector<Point> RetrogradePoints;
		};

		//事件车道缓存
		class EventLaneCache
		{
		public:
			EventLaneCache()
				:LaneIndex(0), LaneId(),LaneType(EventLaneType::None), Region(), XTrend(true), YTrend(true), BaseAsX(false)
				, Congestion(false),LastReportTimeStamp(0), Items()
			{

			}

			//车道序号
			int LaneIndex;
			//车道编号
			std::string LaneId;
			//车道类型
			EventLaneType LaneType;
			//当前检测区域
			Polygon Region;
			//x移动的趋势,true表示正向,逆行检测
			bool XTrend;
			//y移动的趋势,true表示正向,逆行检测
			bool YTrend;
			//判断的坐标系
			bool BaseAsX;

			//当前车道是否处于拥堵状态
			bool Congestion;
			//上一次上报的时间戳,拥堵检测
			long long LastReportTimeStamp;

			//车道内检测项集合
			std::map<std::string, EventDetectCache> Items;

		};

		/**
		* 绘制车辆事件图片
		* @param filePath 写入文件路径
		* @param iveBuffer ive字节流
		* @param detectRegion 检测项区域
		*/
		void DrawDetect(const std::string& filePath,const unsigned char* iveBuffer, const Rectangle& detectRegion);

		//IO mqtt主题
		static const std::string EventTopic;
		//数据移除的时间间隔(毫秒)
		static const int DeleteSpan;
		//最多缓存的检测项缓存数量
		static const int MaxCacheCount;

		//开始计算停车事件的时间长度，单位:毫秒
		static int ParkStartSpan;
		//确认停车事件的时间长度，单位:毫秒
		static int ParkEndSpan;
		//判断拥堵时，区域内的最小机动车数量
		static int CongestionCarCount;
		//上报拥堵的时间间隔,单位:毫秒
		static int CongestionReportSpan;
		//判断车辆逆行点的最小逆行距离，单位：像素
		static double RetrogradeMinMove;
		//确定车辆逆行的最小逆行点的个数
		static unsigned int RetrogradeMinCount;
		//输出视频的I帧数量
		static int OutputVideoIFrame;

		//视频序号
		int _channelIndex;
		//视频地址
		std::string _channelUrl;
		//视频宽度
		int _width;
		//视频高度
		int _height;
		//车道初始化是否成功
		bool _lanesInited;

		//任务编号
		int _taskId;

		//车道集合同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<EventLaneCache> _lanes;
		//等待编码的集合
		std::vector<EventData> _encodeDatas;
		//图像转换
		ImageConvert _image;
		//编码线程
		EncodeChannel* _encodeChannel;
		//事件数据线程
		DataChannel* _dataChannel;
		//视频文件临时存放字节流长度
		int _videoSize;
		//视频文件临时存放字节流
		unsigned char* _videoBuffer;
	};

}

