#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"

namespace OnePunchMan
{
	//车道
	class Lane
	{
	public:
		//通道序号
		int ChannelIndex;
		//车道编号
		std::string LaneId;
		//车道名称
		std::string LaneName;
		//车道序号
		int LaneIndex;
		//车道类型
		int LaneType;
		//车道方向
		int Direction;
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
		//检测线
		std::string DetectLine;
		//停止线
		std::string StopLine;
		//车道线1
		std::string LaneLine1;
		//车道线2
		std::string LaneLine2;
		std::string Region;
	};

	//通道类型
	enum class ChannelType
	{
		GB28181 = 1,
		RTSP = 2,
		File = 3,
		ONVIF = 4
	};

	//设备状态
	enum class DeviceStatus
	{
		Normal = 1,
		Error = 2
	};

	//流量视频通道
	class FlowChannel
	{
	public:
		//通道序号
		int ChannelIndex;
		//通道名称
		std::string ChannelName;
		//通道地址
		std::string ChannelUrl;
		//通道类型
		int ChannelType;
		//通道状态
		int ChannelStatus;
		//车道集合
		std::vector<Lane> Lanes;

		/**
		* @brief: 获取通道rtmp地址
		* @return: 通道rtmp地址
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://",ip,":1935/live/", ChannelIndex);
		}
	};

	//流量视频通道数据库操作
	class FlowChannelData
	{
	public:
		/**
		* @brief: 查询通道列表
		* @return: 通道列表
		*/
		std::vector<FlowChannel> GetList();

		/**
		* @brief: 查询单个通道
		* @param: channelIndex 通道序号
		* @return: 通道
		*/
		FlowChannel Get(int channelIndex);

		/**
		* @brief: 添加通道
		* @param: channel 通道
		* @return: 添加结果
		*/
		bool Insert(const FlowChannel& channel);

		/**
		* @brief: 设置通道
		* @param: channel 通道
		* @return: 设置结果
		*/
		bool Set(const FlowChannel& channel);

		/**
		* @brief: 设置通道集合
		* @param: channels 通道集合
		* @return: 设置结果
		*/
		bool SetList(const std::vector<FlowChannel>& channels);
		
		/**
		* @brief: 删除通道
		* @param: channel 通道
		* @return: 删除结构
		*/
		bool Delete(int channelIndex);

		/**
		* @brief: 清空通道
		*/
		void Clear();

		/**
		* @brief: 获取最后一个错误信息
		* @return: 最后一个错误信息
		*/
		std::string LastError();

		//通道总数
		static const int ChannelCount;

	private:
		/**
		* @brief: 填充通道
		* @param: sqlite 查询结果
		* @return: 通道
		*/
		FlowChannel FillChannel(const SqliteReader& sqlite);

		//数据写入
		SqliteWriter _sqlite;
	};
}


