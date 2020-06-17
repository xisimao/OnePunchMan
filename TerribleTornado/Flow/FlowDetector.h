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
		* @param: channel 通道
		*/
		void UpdateChannel(const FlowChannel& channel);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, int frameIndex,int frameSpan);
		
		void FinishDetect();

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
				, TotalDistance(0.0), TotalTime(0), Speed(0.0)
				, TotalInTime(0), TimeOccupancy(0.0)
				, LastInRegion(0), Vehicles(0), TotalSpan(0), HeadDistance(0.0),HeadSpace(0.0)
				, TrafficStatus(0),IoStatus(false), Flag(false)
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

		//检测报告写入
		class ReportWriter
		{
		public:
			ReportWriter(int channelIndex,int laneCount)
				:_laneCount(laneCount), _minute(0)
			{
				_file.open(StringEx::Combine("../logs/report_", channelIndex, ".txt"), std::ofstream::out);
				_file << std::setiosflags(std::ios::left) << std::setw(10) << "分钟"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "车道"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "轿车"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "卡车"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "客车"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "面包车"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "三轮车"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "自行车"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "摩托车"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "行人"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "平均速度(km/h)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "车头时距(sec)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "车头间距(m)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "时间占有率(%)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "交通状态(1-5)"
					<< std::endl;
				_file.flush();
			}

			~ReportWriter()
			{
				if (_file.is_open())
				{
					_file.close();
				}
			}

			void Write(const FlowLaneCache& cache)
			{
				_file << std::setiosflags(std::ios::left) << std::setw(10) << _minute/_laneCount+1
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.LaneId
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Cars
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Trucks
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Buss
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Vans
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Tricycles
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Bikes
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Motorcycles
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Persons
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.Speed
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.HeadDistance
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.HeadSpace
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.TimeOccupancy
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.TrafficStatus
					<< std::endl;
				_file.flush();
				_minute += 1;
			}

		private:
			//文件流 
			std::ofstream _file;
			//车道数量
			int _laneCount;
			//分钟
			int _minute;
		};

		/**
		* @brief: 计算分钟流量
		* @param: laneCache 车道缓存
		*/
		void CalculateMinuteFlow(FlowLaneCache* laneCache);

		/**
		* @brief: 判断检测项所在车道，如果有车道则发送数据
		* @param: json json字符串
		* @param: recognItem 识别项
		* @param: iveBuffer ive字节流
		*/
		void HandleRecogn(std::string* json,const RecognItem& recognItem, const unsigned char* iveBuffer);

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
		//用于输出检测报告
		ReportWriter* _report;
	};

}

