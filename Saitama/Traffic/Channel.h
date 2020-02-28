#pragma once
#include <queue>
#include <vector>

#include "Lane.h"
#include "Thread.h"

namespace Saitama
{
	//IO状态
	class IOItem
	{
	public:

		/**
		* @brief: 构造函数
		*/
		IOItem()
			:IOItem(std::string(),0, std::string(),0,0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: channelId 通道Id
		* @param: channelIndex 通道编号
		* @param: laneId 车道编号
		* @param: laneIndex 车道序号
		* @param: status 车道IO状态
		*/
		IOItem(const std::string& channelId,int channelIndex,const std::string& laneId,int laneIndex, int status)
			:ChannelId(channelId),ChannelIndex(channelIndex), LaneId(laneId),LaneIndex(laneIndex),Status(status)
		{

		}
		//通道编号
		std::string ChannelId;
		//通道序号
		int ChannelIndex;
		//车道编号
		std::string LaneId;
		//车道序号
		int LaneIndex;
		//IO状态
		int Status;
	};

	//视频通道计算线程
	class Channel
	{
	public:

		/**
		* @brief: 构造函数
		*/
		Channel();

		/**
		* @brief: 析构函数
		*/
		~Channel();

		/**
		* @brief: 更新车道集合
		* @param: lanes 车道集合
		*/
		void UpdateLanes(const std::vector<Lane*>& lanes);

		/**
		* @brief: 检测数据
		* @param: vehicles 机动车检测数据集合
		* @param: bikes 非机动车检测数据集合
		* @param: pedestrains 行人检测数据集合
		* @return: 改变的IO状态集合
		*/
		std::vector<IOItem> Detect(const std::vector<DetectItem>& vehicles,const std::vector<DetectItem>& bikes,std::vector<DetectItem>& pedestrains);

		/**
		* @brief: 收集视频通道计算数据
		* @return: 视频通道计算数据
		*/
		std::vector<LaneItem> Collect();

		//通道编号
		std::string Id;

		//通道序号
		int Index;

	private:

		/**
		* @brief: 清空车道集合
		* @return: 检测区域
		*/
		void ClearLanes();

		//同步锁
		std::mutex _dataMutex;
		//数据队列
		std::queue<std::string> _datas;

		//同步锁
		std::mutex _laneMutex;
		//车道集合
		std::vector<Lane*> _lanes;
	};
}


