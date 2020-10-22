#pragma once
#include <map>
#include <string>
#include <mutex>

#include "Shape.h"
#include "LogPool.h"
#include "MqttChannel.h"

namespace OnePunchMan
{
	//流量报告缓存
	class FlowReportData
	{
	public:
		FlowReportData()
			: ChannelUrl(),LaneId(), TimeStamp(0), ReportProperties(0)
			, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
			, Speed(0.0), TimeOccupancy(0.0), HeadDistance(0.0), HeadSpace(0.0), TrafficStatus(0)
			, QueueLength(0), SpaceOccupancy(0.0)
		{

		}
		//通道地址
		std::string ChannelUrl;
		//车道编号
		std::string LaneId;
		//时间戳
		long long TimeStamp;
		//需要上报的属性
		int ReportProperties;

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

		//排队长度(m)
		int QueueLength;
		//空间占有率(%)
		double SpaceOccupancy;

	};
	
	//数据合并字典
	class DataMergeMap
	{
	public:
		/**
		* 构造函数
		* @param mqtt mqtt
		*/
		DataMergeMap(MqttChannel* mqtt);

		/**
		* 推送流量数据
		* @param data 流量数据
		*/
		void PushData(const FlowReportData& data);

	private:
		//流量mqtt主题
		static const std::string FlowTopic;

		//mqtt
		MqttChannel* _mqtt;

		//同步锁
		std::mutex _mutex;
		//流量数据字典
		std::map<std::string, FlowReportData> _datas;
	};

}


