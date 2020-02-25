#include "Speed.h"

using namespace std;
using namespace Saitama;

void Speed::Collect(const CarItem& item)
{
	//判断是否在范围内
	//一个像素多少米
	const double per = 0.1;
	map<string, CarItem>::iterator it = _cars.find(item.Id);
	if (it != _cars.end())
	{
		//千米
		double distance = item.Region.Top().Distance(it->second.Region.Top())*per/1000.0;
		//小时
		double time=(static_cast<double>(item.TimeStamp)- static_cast<double>(it->second.TimeStamp))/1000.0/3600.0;

		//LogPool::Debug(item.Region.Top().Distance(it->second.Region.Top()) * per, "m ", (static_cast<double>(item.TimeStamp) - static_cast<double>(it->second.TimeStamp)) / 1000.0, "sec ",distance/ time,"km/h");
		_totalDistance += distance;
		_totalTime += time;
	}
	_cars[item.Id] = item;
}

double Speed::Calculate()
{
	double result= _totalDistance / _totalTime;
	_totalDistance = 0.0;
	_totalTime = 0.0;
	return result;
}