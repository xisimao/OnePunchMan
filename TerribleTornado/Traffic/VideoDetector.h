#pragma once
#include <queue>
#include <vector>

#include "FlowChannelData.h"
#include "LaneDetector.h"
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

	//视频数据计算
	class VideoDetector
	{
	public:

		/**
		* @brief: 构造函数
		*/
		VideoDetector();

		/**
		* @brief: 析构函数
		*/
		~VideoDetector();

		/**
		* @brief: 更新车道集合
		* @param: lanes 车道集合
		*/
		void UpdateLanes(const std::vector<Lane>& lanes);

		/**
		* @brief: 检测数据
		* @param: items 检测数据集合
		* @param: timeStamp 时间戳
		* @return: 改变的IO状态集合
		*/
		std::vector<IOItem> Detect(const std::map<std::string,DetectItem>& items,long long timeStamp);

		/**
		* @brief: 检测数据是否在车道范围内
		* @param: item 检测项
		* @return: 在车道内返回车道编号，否则返回空字符串
		*/
		std::string Contains(const DetectItem& item);

		/**
		* @brief: 收集视频通道计算数据
		* @return: 视频通道计算数据
		*/
		std::vector<LaneItem> Collect();

		//通道地址
		std::string Url;

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
		std::vector<LaneDetector*> _lanes;
	};
}


