#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//交通状态
	enum class TrafficStatus
	{
		Good = 1,
		Normal = 2,
		Warning = 3,
		Bad = 4,
		Dead = 5
	};

	//流量车道
	class FlowLane:public TrafficLane
	{
	public:
		FlowLane()
			:TrafficLane(),LaneId(),Direction(0),Region(),DetectLine(),StopLine(),LaneType(0)
			,FlowDirection(0),Length(0),IOIp(),IOPort(0),IOIndex(0), ReportProperties(0)
		{

		}
		//车道编号
		std::string LaneId;
		//车道方向
		int Direction;
		//区域
		std::string Region;
		//检测线
		std::string DetectLine;
		//停止线
		std::string StopLine;
		//车道类型
		int LaneType;
		//车道流向
		int FlowDirection;
		//车道长度
		int Length;
		//io检测器地址
		std::string IOIp;
		//io检测器端口
		int IOPort;
		//io检测器输出口
		int IOIndex;	
		//上报属性
		int ReportProperties;
	};

	//流量视频通道
	class FlowChannel:public TrafficChannel
	{
	public:
		//车道集合
		std::vector<FlowLane> Lanes;
	};

	//流量视频通道数据库操作
	class FlowChannelData:public TrafficData
	{
	public:
		/**
		* 查询通道列表
		* @return 通道列表
		*/
		std::vector<FlowChannel> GetList();

		/**
		* 查询车道列表
		* @param channelIndex 通道序号
		* @param laneId 车道编号
		* @return 车道列表
		*/
		std::vector<FlowLane> GetLaneList(int channelIndex,const std::string& laneId);

		/**
		* 查询单个通道
		* @param channelIndex 通道序号
		* @return 通道
		*/
		FlowChannel Get(int channelIndex);

		/**
		* 设置通道
		* @param channel 通道
		* @return 设置结果
		*/
		bool Set(const FlowChannel& channel);

		/**
		* 设置通道集合
		* @param channels 通道集合
		* @return 设置结果
		*/
		bool SetList(const std::vector<FlowChannel>& channels);
		
		/**
		* 删除通道
		* @param channel 通道
		* @return 删除结果
		*/
		bool Delete(int channelIndex);

		/**
		* 清空通道
		*/
		void Clear();

		void UpdateDb();

	private:
		/**
		* 添加通道
		* @param channel 通道
		* @return 添加结果
		*/
		bool InsertChannel(const FlowChannel& channel);

		/**
		* 添加车道
		* @param lane 车道
		* @return 添加结果
		*/
		bool InsertLane(const FlowLane& lane);

		/**
		* 填充通道
		* @param sqlite 查询结果
		* @return 通道
		*/
		FlowChannel FillChannel(const SqliteReader& sqlite);

		/**
		* 填充车道
		* @param sqlite 查询结果
		* @return 车道
		*/
		FlowLane FillLane(const SqliteReader& sqlite);
	};

	//流量报告数据
	class FlowReportData
	{
	public:
		FlowReportData()
			: ChannelUrl(),LaneId(), LaneName(), Direction(0), ReportProperties(0)
			, Minute(0), TimeStamp(0)
			, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
			, Speed(0.0), TimeOccupancy(0.0), HeadDistance(0.0), HeadSpace(0.0), TrafficStatus(0)
			, QueueLength(0), SpaceOccupancy(0.0)
		{

		}
		//通道地址
		std::string ChannelUrl;
		//车道编号
		std::string LaneId;
		//车道名称
		std::string LaneName;
		//车道方向
		int Direction;
		//需要上报的属性
		int ReportProperties;

		//第几分钟
		int Minute;
		//时间戳
		long long TimeStamp;

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

		/**
		* 获取数据显示的字符串
		* @return 数据显示的字符串
		*/
		std::string ToString()
		{
			return StringEx::Combine("channel url:", ChannelUrl, " lane:", LaneId, " timestamp:", DateTime::ParseTimeStamp(TimeStamp).ToString(), " properties:", ReportProperties, " cars:", Cars, " tricycles:", Tricycles, " buss", Buss, " vans:", Vans, " trucks:", Trucks, " bikes:", Bikes, " motorcycles:", Motorcycles, " persons:", Persons, " speed(km/h):", Speed, " head distance(sec):", HeadDistance, " head space(m)", HeadSpace, " time occ(%):", TimeOccupancy, " traffic status:", TrafficStatus, " queue length(m):", QueueLength, " pace occ(%):", SpaceOccupancy);
		}

		/**
		* 获取excel用的json
		* @return excel用的json
		*/
		std::string ToReportJson()
		{
			std::string reportJson;
			JsonSerialization::SerializeValue(&reportJson, "minute", Minute);
			JsonSerialization::SerializeValue(&reportJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&reportJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&reportJson, "direction", Direction);
			JsonSerialization::SerializeValue(&reportJson, "persons", Persons);
			JsonSerialization::SerializeValue(&reportJson, "bikes", Bikes);
			JsonSerialization::SerializeValue(&reportJson, "motorcycles", Motorcycles);
			JsonSerialization::SerializeValue(&reportJson, "cars", Cars);
			JsonSerialization::SerializeValue(&reportJson, "tricycles", Tricycles);
			JsonSerialization::SerializeValue(&reportJson, "buss", Buss);
			JsonSerialization::SerializeValue(&reportJson, "vans", Vans);
			JsonSerialization::SerializeValue(&reportJson, "trucks", Trucks);
			JsonSerialization::SerializeValue(&reportJson, "averageSpeed", static_cast<int>(Speed));
			JsonSerialization::SerializeValue(&reportJson, "headDistance", HeadDistance);
			JsonSerialization::SerializeValue(&reportJson, "headSpace", HeadSpace);
			JsonSerialization::SerializeValue(&reportJson, "timeOccupancy", static_cast<int>(TimeOccupancy));
			JsonSerialization::SerializeValue(&reportJson, "trafficStatus", TrafficStatus);
			JsonSerialization::SerializeValue(&reportJson, "queueLength", QueueLength);
			JsonSerialization::SerializeValue(&reportJson, "spaceOccupancy", SpaceOccupancy);
			return reportJson;
		}

		/**
		* 获取mqtt报告用的json
		* @return mqtt报告用的json
		*/
		std::string ToMessageJson()
		{
			std::string messageJson;
			JsonSerialization::SerializeValue(&messageJson, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&messageJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&messageJson, "timeStamp", TimeStamp);

			JsonSerialization::SerializeValue(&messageJson, "persons", Persons);
			JsonSerialization::SerializeValue(&messageJson, "bikes", Bikes);
			JsonSerialization::SerializeValue(&messageJson, "motorcycles", Motorcycles);
			JsonSerialization::SerializeValue(&messageJson, "cars", Cars);
			JsonSerialization::SerializeValue(&messageJson, "tricycles", Tricycles);
			JsonSerialization::SerializeValue(&messageJson, "buss", Buss);
			JsonSerialization::SerializeValue(&messageJson, "vans", Vans);
			JsonSerialization::SerializeValue(&messageJson, "trucks", Trucks);

			JsonSerialization::SerializeValue(&messageJson, "averageSpeed", static_cast<int>(Speed));
			JsonSerialization::SerializeValue(&messageJson, "headDistance", HeadDistance);
			JsonSerialization::SerializeValue(&messageJson, "headSpace", HeadSpace);
			JsonSerialization::SerializeValue(&messageJson, "timeOccupancy", static_cast<int>(TimeOccupancy));
			JsonSerialization::SerializeValue(&messageJson, "trafficStatus", TrafficStatus);

			JsonSerialization::SerializeValue(&messageJson, "queueLength", QueueLength);
			JsonSerialization::SerializeValue(&messageJson, "spaceOccupancy", SpaceOccupancy);
			return messageJson;
		}
	};
}


