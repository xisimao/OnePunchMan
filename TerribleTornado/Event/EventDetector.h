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

	//�������ݼ���
	class EventDetector
	{
	public:

		/**
		* @brief: ���캯��
		* @param: lane ����
		*/
		EventDetector(const EventLane& lane);

		/**
		* @brief: ���
		* @param: items ����������
		* @param: timeStamp ʱ���
		* @return: �����¼����������
		*/
		std::vector<EventResult> Detect(std::map<std::string, DetectItem>* items, long long timeStamp);

	private:
		bool _inited;

		//�������
		std::string _laneId;

		Polygon _pedestrainRegion;

		std::vector<Polygon> _parkRegions;
		//ͬ����
		std::mutex _mutex;
		std::map<std::string, DetectItem> _items1;
		std::map<std::string, DetectItem> _items2;
		std::map<std::string, DetectItem>* _currentItems;
		std::map<std::string, DetectItem>* _lastItems;
	};

}

