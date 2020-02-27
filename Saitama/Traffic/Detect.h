#pragma once
#include <string>

#include "Shape.h"

namespace Saitama
{
	//检测项
	class DetectItem
	{
	public:

		/**
		* @brief: 构造函数
		*/
		DetectItem()
			:DetectItem(std::string(), 0, Rectangle())
		{

		}

		/**
		* @brief: 构造函数
		* @param: id 检测元素编号
		* @param: timeStamp 时间戳
		* @param: region 检测元素区域
		*/
		DetectItem(const std::string& id, int timeStamp, const Rectangle& region)
			:Id(id), TimeStamp(timeStamp), Region(region)
		{

		}

		//检测元素编号
		std::string Id;
		//时间戳
		long long TimeStamp;
		//检测元素区域
		Rectangle Region;
	};

	//检测元素类型
	enum class DetectType
	{
		Pedestrain = 1,
		Bike = 2,
		Motobike = 3,
		Car = 4,
		Tricycle = 5,
		Bus = 6,
		Van = 7,
		Truck = 8
	};
}