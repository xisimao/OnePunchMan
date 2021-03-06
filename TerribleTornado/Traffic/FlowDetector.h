#pragma once
#include <list>

#include "ImageConvert.h"
#include "TrafficData.h"
#include "Command.h"
#include "DataChannel.h"
#include "SocketMaid.h"
#include "ImageDrawing.h"


namespace OnePunchMan
{
	//通道检测
	class FlowDetector 
	{
	public:
		/**
		* 构造函数
		* @param width 图片宽度
		* @param height 图片高度
		* @param maid socket
		* @param data 数据线程
		*/
		FlowDetector(int width, int height,SocketMaid* maid,DataChannel* data);

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
		* 获取车道是否初始化成功
		* @return 车道是否初始化成功
		*/
		bool LanesInited() const;

		/**
		* 重置时间范围
		*/
		void ResetTimeRange();

		/**
		* 获取报告json数据
		* @param json 用于存放json数据的字符串
		*/
		void GetReportJson(std::string* json);

		/**
		* 获取io数据集合
		* @param laneId 车道编号，为空时查询所有
		* @return io数据集合
		*/
		std::vector<IoData> GetIoDatas(const std::string& laneId);

		/**
		* 获取IO状态
		* @param laneId 车道编号，为空时查询所有
		* @return io状态集合
		*/
		std::vector<IoData> GetIoStatus(const std::string& laneId);

		/**
		* 流量检测
		* @param items 检测数据项集合
		* @param timeStamp 时间戳
		* @param taskId 任务编号
		* @param frameIndex 帧序号
		* @param frameSpan 帧间隔时间(毫秒)
		*/
		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp,unsigned char taskId, unsigned int frameIndex,unsigned char frameSpan);
		
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
		void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, VehicleData* vehicle);

		/**
		* 处理非机动车识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param bike 非机动车识别数据
		*/
		void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, BikeData* bike);

		/**
		* 处理行人识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param pedestrain 行人识别数据
		*/
		void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, PedestrainData* pedestrain);
		
		/**
		* 绘制检测区域
		* @param detectItems 检测项集合
		* @param frameIndex 帧序号
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, unsigned int frameIndex);

	private:
		/**
		* 计算分钟流量
		* @param laneCache 车道缓存
		* @return 分钟流量数据
		*/
		FlowData CalculateMinuteFlow(FlowData* laneCache);

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
		int CalculateQueueLength(const FlowData& laneCache,const std::list<double>& distances);

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
		//准备计算二次排队的瞬时车头时距(ms)
		static int ReadyCalculateQueueCarSpan;
		//等待计算排队的间隔时间(ms)
		static int WaitCalculateQueueSpan;

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

		//socket
		SocketMaid* _maid;
		//数据处理线程
		DataChannel* _data;

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
		std::vector<FlowData> _laneCaches;
		//上报缓存
		std::vector<FlowData> _reportCaches;
		std::vector<VehicleData> _vehicleReportCaches;
		std::vector<BikeData> _bikeReportCaches;
		std::vector<PedestrainData> _pedestrainReportCaches;

		//io数据缓存
		std::map<std::string,std::list<IoData>> _ioDatas;

		//监测图像转换
		ImageConvert _detectImage;
		//识别图像转换
		ImageConvert _recognImage;

		//是否输出检测报告
		bool _outputReport;
		//当前报告的分钟
		int _currentReportMinute;
		//是否输出识别项
		bool _outputRecogn;

	};

}

