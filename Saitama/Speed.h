#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"

namespace Saitama
{
	class CarItem
	{
	public:
		CarItem()
			:CarItem(std::string(),0,0,0,0,0)
		{

		}

		CarItem(const std::string& id,int timeStamp,int x,int y,int width,int height)
			:Id(id),TimeStamp(timeStamp), Region(x,y,width,height)
		{

		}

		std::string Id;
		long long TimeStamp;
		Rectangle Region;
	};

	class Speed
	{
	public:

		void Collect(const CarItem& item);

		double Calculate();

	private:

		std::map<std::string, CarItem> _cars;
		double _totalDistance;
		double _totalTime;
	};

}

