#pragma once
#include <list>

#include "TrafficData.h"
#include "TrafficDetector.h"
#include "Command.h"
#include "DataMergeMap.h"

namespace OnePunchMan
{
	//通道检测
	class FlowDetector :public TrafficDetector
	{
	public:
		/**
		* 构造函数
		* @param width 图片宽度
		* @param height 图片高度
		* @param mqtt mqtt
		* @param merge 数据合并
		*/
		FlowDetector(int width, int height,MqttChannel* mqtt, DataMergeMap* merge);
		
		/**
		* 析构函数
		*/
		~FlowDetector();

		/**
		* 初始化流量参数配置
		* @param jd json配置
		*/
		static void Init(const JsonDeserialization& jd);

		/**
		* 更新通道
		* @param taskId 任务编号
		* @param channel 通道
		*/
		void UpdateChannel(const unsigned char taskId,const TrafficChannel& channel);

		/**
		* 清空通道
		*/
		void ClearChannel();

		/**
		* 获取报告json数据
		* @param json 用于存放json数据的字符串
		*/
		void GetReportJson(std::string* json);

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp,unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex,unsigned char frameSpan);
		
		/**
		 * 结束检测
		 * @param taskId 任务编号
		 */
		void FinishDetect(unsigned char taskId);

		/**
		* 处理机动车识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param vehicle 机动车识别数据
		*/
		void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle);

		/**
		* 处理非机动车识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param bike 非机动车识别数据
		*/
		void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike);

		/**
		* 处理行人识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param pedestrain 行人识别数据
		*/
		void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain);

	private:
		//流量检测缓存
		class FlowDetectCache
		{
		public:
			FlowDetectCache()
				:LastTimeStamp(0), LastHitPoint()
			{

			}
			//检测项最后一次出现的时间戳
			long long LastTimeStamp;
			//检测项最后一次检测点
			Point LastHitPoint;
		};

		//流量车道缓存
		class FlowLaneCache
		{
		public:
			FlowLaneCache()
				: LaneId(), LaneName(), Length(0), Direction(), FlowRegion(), QueueRegion(),ReportProperties(0), MeterPerPixel(0.0), StopPoint()
				, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
				, TotalDistance(0.0), TotalTime(0)
				, TotalInTime(0)
				, LastInRegion(0), TotalSpan(0)
				, MaxQueueLength(0), CurrentQueueLength(0), TotalQueueLength(0), CountQueueLength(0)
				, IoStatus(false), Items()
			{

			}
			//车道编号
			std::string LaneId;
			//车道名称
			std::string LaneName;
			//车道长度
			int Length;
			//车道方向
			int Direction;
			//流量检测区域
			Polygon FlowRegion;
			//排队检测区域
			Polygon QueueRegion;
			//上报的属性
			int ReportProperties;
			//每个像素代表的米数
			double MeterPerPixel;
			//停止线检测的点
			Point StopPoint;

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

			//用于计算时间占有率
			//区域占用总时间(毫秒)
			long long TotalInTime;

			//用于计算车头时距
			//上一次有车进入区域的时间戳 
			long long LastInRegion;
			//车辆进入区域时间差的和(毫秒)
			long long TotalSpan;

			//排队长度(m)
			int MaxQueueLength;
			//当前瞬时排队长度(m)
			int CurrentQueueLength;
			//排队总长度(m)
			int TotalQueueLength;
			//排队总长度对应的计数次数
			int CountQueueLength;

			//io状态
			bool IoStatus;

			//车道内检测项集合
			std::map<std::string, FlowDetectCache> Items;
		};

		/**
		* 计算分钟流量
		* @param laneCache 车道缓存
		* @return 分钟流量数据
		*/
		FlowReportData CalculateMinuteFlow(FlowLaneCache* laneCache);

		/**
		* 添加车辆距离有序列表
		* @param distances 车辆距离链表
		* @param distance 车辆距离停止线长度(px)
		*/
		void AddOrderedList(std::list<double>* distances,double distance);

		/**
		* 计算排队长度
		* @param laneCache 流量车道缓存
		* @param distances 车辆距离链表
		* @return 车辆排队长度(m)
		*/
		int CalculateQueueLength(const FlowLaneCache& laneCache,const std::list<double>& distances);

		/**
		* 绘制检测区域
		* @param detectItems 检测项集合
		* @param hasIoChanged io是否变化
		* @param hasNewCar 是否有新车
		* @param hasQueue 是否有排队
		* @param iveBuffer ive字节流
		* @param frameIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems,bool hasIoChanged,bool hasNewCar,bool hasQueue, const unsigned char* iveBuffer, unsigned int frameIndex);

		//IO mqtt主题
		static const std::string IOTopic;
		//流量mqtt主题
		static const std::string FlowTopic;
		//视频结构化mqtt主题
		static const std::string VideoStructTopic;
		//上报的最大时长(毫秒)
		static const int ReportMaxSpan;
		//数据移除的时间间隔(毫秒)
		static const int DeleteSpan;
		//计算两车是否处于排队状态的最小距离(px)
		static int QueueMinDistance;
		//车道上报所有流量属性的标识
		static const int AllPropertiesFlag;

		//数据合并
		DataMergeMap* _merge;

		//任务编号
		int _taskId;

		//上一帧的时间戳
		long long _lastFrameTimeStamp;
		//当前分钟的时间戳
		long long _currentMinuteTimeStamp;
		//下一分钟的时间戳
		long long _nextMinuteTimeStamp;

		//检测车道集合同步锁
		std::mutex _laneMutex;
		//检测车道集合
		std::vector<FlowLaneCache> _laneCaches;
		//上报缓存
		std::vector<FlowReportData> _reportCaches;
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

