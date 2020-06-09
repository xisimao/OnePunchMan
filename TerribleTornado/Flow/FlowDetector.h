#pragma once
#include "FlowData.h"
#include "TrafficDetector.h"
#include "ImageConvert.h"

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
		* @param: debug 是否处于调试模式，处于调试模式则输出画线后的bmp
		*/
		FlowDetector(int width, int height,MqttChannel* mqtt, bool debug);

		/**
		* @brief: 更新通道
		* @param: channel 通道
		*/
		void UpdateChannel(const FlowChannel& channel);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, const unsigned char* yuvBuffer,int frameIndex,int frameSpan);
		
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
				: LaneId(), Region(),MeterPerPixel(0.0)
				, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
				, TotalDistance(0.0), TotalTime(0)
				, TotalInTime(0)
				, LastInRegion(0), Vehicles(0), TotalSpan(0)
				, IoStatus(false), Flag(false)
			{

			}

			//车道编号
			std::string LaneId;
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

			//用于计算时间占有率
			//区域占用总时间(毫秒)
			long long TotalInTime;

			//用于计算车头时距
			//上一次有车进入区域的时间戳 
			long long LastInRegion;
			//机动车总数
			int Vehicles;
			//车辆进入区域时间差的和(毫秒)
			long long TotalSpan;

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

		bool ContainsRecogn(std::string* json,const RecognItem& recognItem, const unsigned char* iveBuffer);

		/**
		* @brief: 绘制检测区域
		* @param: detectItems 检测项集合
		* @param: iveBuffer ive字节流
		* @param: frameIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex);

		//IO mqtt主题
		static const std::string IOTopic;
		//流量mqtt主题
		static const std::string FlowTopic;
		//视频结构化mqtt主题
		static const std::string VideoStructTopic;
		//上报的最大时长(毫秒)
		static const int ReportMaxSpan;

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

	};

}

