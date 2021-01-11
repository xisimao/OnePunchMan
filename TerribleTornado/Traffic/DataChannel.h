#pragma once
#include "Thread.h"
#include "Sqlite.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//事件数据操作线程
	class DataChannel:public ThreadObject
	{
	public:
		/**
		* 构造函数
		*/
		DataChannel();

		/**
		* 初始化配置参数
		* @param jd 配置参数
		*/
		static void Init(const JsonDeserialization& jd);

		/**
		* 添加新的事件数据
		* @param data 事件数据
		*/
		void PushEventData(const EventData& data);

		/**
		* 添加新的事件数据
		* @param data 事件数据
		*/
		void PushFlowData(const FlowData& data);

		/**
		* 添加新的事件数据
		* @param data 事件数据
		*/
		void PushVehicleData(const VehicleData& data);

		/**
		* 添加新的事件数据
		* @param data 事件数据
		*/
		void PushBikeData(const BikeData& data);

		/**
		* 添加新的事件数据
		* @param data 事件数据
		*/
		void PushPedestrainData(const PedestrainData& data);

	protected:
		void StartCore();

	private:
		//最多保存的事件数据数量
		static int MaxDataCount;

		//流量队列
		std::mutex _flowMutex;
		//部分的流量数据字典
		std::map<std::string, FlowData> _mergeFlowDatas;
		//完整的流量数据队列
		std::vector<FlowData> _flowDatas;

		//机动车队列
		std::mutex _vehicleMutex;
		std::vector<VehicleData> _vehicleDatas;

		//非机动车队列
		std::mutex _bikeMutex;
		std::vector<BikeData> _bikeDatas;

		//行人队列
		std::mutex _pedestrainMutex;
		std::vector<PedestrainData> _pedestrainDatas;

		//时间队列
		std::mutex _eventMutex;
		std::vector<EventData> _eventDatas;
	};

}

