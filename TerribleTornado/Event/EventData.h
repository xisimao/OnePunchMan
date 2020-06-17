#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//事件车道类型
	enum class EventLaneType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Lane = 3
	};

	//事件车道
	class EventLane:public TrafficLane
	{
	public:
		//车道类型
		int LaneType;
		//箭头
		std::string Line;
	};

	//事件视频通道
	class EventChannel:public TrafficChannel
	{
	public:
		//车道集合
		std::vector<EventLane> Lanes;

	};

	//流量视频通道数据库操作
	class EventChannelData:public TrafficData
	{
	public:
		/**
		* @brief: 查询通道列表
		* @return: 通道列表
		*/
		std::vector<EventChannel> GetList();

		/**
		* @brief: 查询单个通道
		* @param: channelIndex 通道序号
		* @return: 通道
		*/
		EventChannel Get(int channelIndex);

		/**
		* @brief: 添加通道
		* @param: channel 通道
		* @return: 添加结果
		*/
		bool Insert(const EventChannel& channel);

		/**
		* @brief: 设置通道
		* @param: channel 通道
		* @return: 设置结果
		*/
		bool Set(const EventChannel& channel);

		/**
		* @brief: 设置通道集合
		* @param: channels 通道集合
		* @return: 设置结果
		*/
		bool SetList(const std::vector<EventChannel>& channels);
		
		/**
		* @brief: 删除通道
		* @param: channel 通道
		* @return: 删除结果
		*/
		bool Delete(int channelIndex);

		/**
		* @brief: 清空通道
		*/
		void Clear();

		void UpdateDb();

	private:
		/**
		* @brief: 填充通道
		* @param: sqlite 查询结果
		* @return: 通道
		*/
		EventChannel FillChannel(const SqliteReader& sqlite);
	};
}


