#pragma once
#include <map>
#include <string>
#include <mutex>

#include "Shape.h"
#include "LogPool.h"
#include "MqttChannel.h"
#include "FlowData.h"

namespace OnePunchMan
{
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


