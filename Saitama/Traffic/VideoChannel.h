#pragma once
#include <queue>
#include <vector>

#include "Lane.h"
#include "Thread.h"

namespace Saitama
{
	//视频通道计算线程
	class VideoChannel:public ThreadObject
	{
	public:

		/**
		* @brief: 构造函数
		*/
		VideoChannel();

		/**
		* @brief: 析构函数
		*/
		~VideoChannel();

		/**
		* @brief: 更新车道集合
		* @param: lanes 车道集合
		*/
		void UpdateLanes(const std::vector<Lane*>& lanes);

		/**
		* @brief: 推送数据
		* @param: data 视频通道检测数据
		*/
		void Push(const std::string& data);

		/**
		* @brief: 收集视频通道计算数据
		* @return: 视频通道计算数据
		*/
		std::string Collect();

	protected:

		void StartCore();

		/**
		* @brief: 从检测数据中获取检测区域
		* @return: 检测区域
		*/
		Rectangle GetRegion(const std::string& data);

		/**
		* @brief: 清空车道集合
		* @return: 检测区域
		*/
		void ClearLanes();

	private:

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


