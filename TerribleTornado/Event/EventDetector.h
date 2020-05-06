#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"
#include "EventData.h"

namespace OnePunchMan
{
	enum class EventType
	{
		Pedestrain,
		Park
	};

	class EventResult
	{
	public:
		std::string LaneId;
		EventType Type;
	};

	//车道数据计算
	class EventDetector
	{
	public:

		/**
		* @brief: 构造函数
		* @param: lane 车道
		*/
		EventDetector(const EventLane& lane);

		/**
		* @brief: 检测
		* @param: items 检测数据项集合
		* @param: timeStamp 时间戳
		* @return: 返回事件监测结果集合
		*/
		std::vector<EventResult> Detect(std::map<std::string, DetectItem>* items, long long timeStamp);

	private:
		bool _inited;

		//车道编号
		std::string _laneId;

		Polygon _pedestrainRegion;

		std::vector<Polygon> _parkRegions;
		//同步锁
		std::mutex _mutex;
		std::map<std::string, DetectItem> _items1;
		std::map<std::string, DetectItem> _items2;
		std::map<std::string, DetectItem>* _currentItems;
		std::map<std::string, DetectItem>* _lastItems;
	};

}

