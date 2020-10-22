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
}


