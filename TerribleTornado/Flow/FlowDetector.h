#pragma once
#include "FlowData.h"
#include "TrafficDetector.h"
#include "Command.h"

namespace OnePunchMan
{
	//通道检测
	class FlowDetector :public TrafficDetector
	{
	public:
		/**
		* @brief: 构造函数
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: mqtt mqtt
		*/
		FlowDetector(int width, int height,MqttChannel* mqtt);
		
		/**
		* @brief: 析构函数
		*/
		~FlowDetector();

		/**
		* @brief: 更新通道
		* @param: taskId 任务编号
		* @param: channel 通道
		*/
		void UpdateChannel(unsigned char taskId,const FlowChannel& channel);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

		/**
		* @brief: 获取报告json数据
		* @param: json 用于存放json数据的字符串
		*/
		void GetReportJson(std::string* json);

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex,unsigned char frameSpan);
		
		void FinishDetect(unsigned char taskId);

		void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle);
		
		void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike);
		
		void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain);

	private:
		//流量检测缓存
		class FlowDetectCache
		{
		public:
			FlowDetectCache()
				:HitPoint()
			{

			}

			//检测点
			Point HitPoint;
		};

		//流量车道缓存
		class FlowLaneCache
		{
		public:
			FlowLaneCache()
				: LaneId(),LaneName(), Direction(), Region(),MeterPerPixel(0.0)
				, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
				, TotalDistance(0.0), TotalTime(0), Speed(0.0)
				, TotalInTime(0), TimeOccupancy(0.0)
				, LastInRegion(0), Vehicles(0), TotalSpan(0), HeadDistance(0.0),HeadSpace(0.0)
				, TrafficStatus(0),IoStatus(false), Flag(false)
			{

			}

			//车道编号
			std::string LaneId;
			//车道名称
			std::string LaneName;
			//车道方向
			int Direction;
			//当前检测区域
			Polygon Region;
			//每个像素代表的米数
			double MeterPerPixel;

			//行人流量
			int Persons;
			//自行车流量
			int Bikes;
			//摩托车流量
			int Motorcycles;
			//轿车流量
			int Cars;
			//三轮车流量
			int Tricycles;
			//公交车流量
			int Buss;
			//面包车流量
			int Vans;
			//卡车流量
			int Trucks;

			//用于计算平均速度
			//车辆行驶总距离(像素值) 
			double TotalDistance;
			//车辆行驶总时间(毫秒)
			long long TotalTime;
			//平均速度(km/h)
			double Speed;

			//用于计算时间占有率
			//区域占用总时间(毫秒)
			long long TotalInTime;
			//时间占用率(%)
			double TimeOccupancy;

			//用于计算车头时距
			//上一次有车进入区域的时间戳 
			long long LastInRegion;
			//机动车总数
			int Vehicles;
			//车辆进入区域时间差的和(毫秒)
			long long TotalSpan;
			//车道时距(sec)
			double HeadDistance;
			//车头间距(m)
			double HeadSpace;

			//交通状态
			int TrafficStatus;

			//io状态
			bool IoStatus;

			//指针指向实际位置的交替变化标志
			bool Flag;
			//车道内检测项集合
			std::map<std::string, FlowDetectCache> Items1;
			std::map<std::string, FlowDetectCache> Items2;

			/**
			* @brief: 获取本次检测项的集合指针
			* @return: 本次检测项的集合指针
			*/
			std::map<std::string, FlowDetectCache>* CurrentItems()
			{
				return Flag ? &Items1 : &Items2;
			}

			/**
			* @brief: 获取上一次检测项的集合指针
			* @return: 上一次检测项的集合指针
			*/
			std::map<std::string, FlowDetectCache>* LastItems()
			{
				return Flag ? &Items2 : &Items1;
			}

			/**
			* @brief: 交换当前和上一次的指针
			*/
			void SwitchFlag()
			{
				Flag = !Flag;
			}
		};

		//流量报告缓存
		class FlowReportCache
		{
		public:
			FlowReportCache()
				: LaneId(),LaneName(), Direction(0), Minute(0)
				, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
				, Speed(0.0), TimeOccupancy(0.0), HeadDistance(0.0), HeadSpace(0.0), TrafficStatus(0)
			{

			}

			//车道编号
			std::string LaneId;
			//车道名称
			std::string LaneName;
			//车道方向
			int Direction;
			//第几分钟
			int Minute;
			//行人流量
			int Persons;
			//自行车流量
			int Bikes;
			//摩托车流量
			int Motorcycles;
			//轿车流量
			int Cars;
			//三轮车流量
			int Tricycles;
			//公交车流量
			int Buss;
			//面包车流量
			int Vans;
			//卡车流量
			int Trucks;

			//平均速度(km/h)
			double Speed;
			//时间占用率(%)
			double TimeOccupancy;
			//车道时距(sec)
			double HeadDistance;
			//车头间距(m)
			double HeadSpace;
			//交通状态
			int TrafficStatus;
		};

		/**
		* @brief: 计算分钟流量
		* @param: laneCache 车道缓存
		*/
		void CalculateMinuteFlow(FlowLaneCache* laneCache);

		/**
		* @brief: 绘制检测区域
		* @param: detectItems 检测项集合
		* @param: ioChanged 是否是io变化的帧
		* @param: iveBuffer ive字节流
		* @param: frameIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, unsigned int frameIndex);

		//IO mqtt主题
		static const std::string IOTopic;
		//流量mqtt主题
		static const std::string FlowTopic;
		//视频结构化mqtt主题
		static const std::string VideoStructTopic;
		//上报的最大时长(毫秒)
		static const int ReportMaxSpan;

		//任务编号
		int _taskId;

		//上一帧的时间戳
		long long _lastFrameTimeStamp;
		//当前分钟的时间戳
		long long _currentMinuteTimeStamp;
		//下一分钟的时间戳
		long long _nextMinuteTimeStamp;

		//检测车道集合同步锁
		std::mutex _detectLaneMutex;
		//检测车道集合
		std::vector<FlowLaneCache> _detectLanes;
		//识别车道集合同步锁
		std::mutex _recognLaneMutex;
		//通道地址
		std::string _recognChannelUrl;
		//识别车道集合
		std::vector<FlowLaneCache> _recognLanes;

		//上报缓存
		std::mutex _reportMutex;
		std::vector<FlowReportCache> _reportCaches;
		std::vector<VideoStruct_Vehicle> _vehicleReportCaches;
		std::vector<VideoStruct_Bike> _bikeReportCaches;
		std::vector<VideoStruct_Pedestrain> _pedestrainReportCaches;

		//监测时用到的bgr字节流
		unsigned char* _detectBgrBuffer;
		//监测时用到的jpg字节流
		unsigned char* _detectJpgBuffer;
		//识别时用到的bgr字节流
		unsigned char* _recognBgrBuffer;
		//识别时用到的jpg字节流
		unsigned char* _recognJpgBuffer;

		//是否输出图片
		bool _outputImage;
		//是否输出检测报告
		bool _outputReport;
		//当前报告的分钟
		int _currentReportMinute;
		//是否输出识别项
		bool _outputRecogn;
	};

}

