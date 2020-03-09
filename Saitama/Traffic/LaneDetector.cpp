#include "LaneDetector.h"

using namespace std;
using namespace Saitama;

LaneDetector::LaneDetector(const string& id,int index,Polygon region)
	:_id(id),_index(index),_region(region),Status(false), _persons(0),_bikes(0), _motorcycles(0), _cars(0),_tricycles(0), _buss(0),_vans(0),_trucks(0),  _totalDistance(0.0), _totalTime(0), _lastInRegion(0), _vehicles(0), _totalSpan(0)
{

}

string LaneDetector::Id()
{
	return _id;
}

int LaneDetector::Index()
{
	return _index;
}

LaneItem LaneDetector::Collect()
{
	std::lock_guard<std::mutex> lck(_mutex);
	LaneItem item;
	item.Id = _id;
	item.Index = _index;

	item.Persons = _persons;
	item.Bikes = _bikes;
	item.Motorcycles = _motorcycles;
	item.Tricycles = _tricycles;
	item.Trucks = _trucks;
	item.Vans = _vans;
	item.Cars = _cars;
	item.Buss = _buss;

	const double per = 0.1;
	item.Speed = (_totalDistance * per / 1000.0) / (_totalTime / 3600000.0);
	item.HeadDistance = _vehicles > 1 ? _totalSpan / (static_cast<long long>(_vehicles) - 1) / 1000.0 : 0;
	item.TimeOccupancy = _totalTime / 60000.0 * 100;

	_persons = 0;
	_bikes = 0;
	_motorcycles = 0;
	_tricycles = 0;
	_trucks = 0;
	_vans = 0;
	_cars = 0;
	_buss = 0;

	_totalDistance = 0.0;
	_totalTime = 0;

	_lastInRegion = 0;
	_vehicles = 0;
	_totalSpan = 0;

	_items.clear();
	return item;
}

bool LaneDetector::DetectVehicle(const DetectItem& item)
{
	if (Contains(item))
	{
		std::lock_guard<std::mutex> lck(_mutex);
		map<string, DetectItem>::iterator mit = _items.find(item.Id);

		//如果是新车则计算流量和车头时距
		//流量=车数量
		//车头时距=所有进入区域的时间差的平均值
		if (mit == _items.end())
		{
			if (item.Type == (int)DetectType::Car)
			{
				_cars += 1;
			}
			else if (item.Type == (int)DetectType::Tricycle)
			{
				_tricycles += 1;
			}
			else if (item.Type == (int)DetectType::Bus)
			{
				_buss += 1;
			}
			else if (item.Type == (int)DetectType::Van)
			{
				_vans += 1;
			}
			else if (item.Type == (int)DetectType::Truck)
			{
				_trucks += 1;
			}
			_vehicles += 1;

			if (_lastInRegion != 0)
			{
				_totalSpan += item.TimeStamp - _lastInRegion;
			}
			_lastInRegion = item.TimeStamp;

		}
		//如果是已经记录的车则计算平均速度和时间占有率
		//平均速度=总距离/总时间
		//总距离=两次检测到的点的距离*每个像素代表的米数
		//总时间=两次检测到的时间戳时长
		//时间占有率=总时间/一分钟
		else
		{
			_totalDistance += item.Region.HitPoint().Distance(mit->second.Region.HitPoint());
			_totalTime += item.TimeStamp - mit->second.TimeStamp;
		}
		_items[item.Id] = item;
		return true;
	}
	else
	{
		return false;
	}
}

bool LaneDetector::DetectBike(const DetectItem& item)
{
	if (Contains(item))
	{
		std::lock_guard<std::mutex> lck(_mutex);
		map<string, DetectItem>::iterator mit = _items.find(item.Id);
		if (mit == _items.end())
		{
			if (item.Type == (int)DetectType::Bike)
			{
				_bikes += 1;
			}
			else if (item.Type == (int)DetectType::Motobike)
			{
				_motorcycles += 1;
			}
		}
		_items[item.Id] = item;
		return true;
	}
	else
	{
		return false;
	}
}

bool LaneDetector::DetectPedestrain(const DetectItem& item)
{
	if (Contains(item))
	{
		std::lock_guard<std::mutex> lck(_mutex);
		map<string, DetectItem>::iterator mit = _items.find(item.Id);
		if (mit == _items.end())
		{
			_persons += 1;
		}
		_items[item.Id] = item;
		return true;
	}
	else
	{
		return false;
	}
}

bool LaneDetector::Contains(const DetectItem& item)
{
	return _region.Contains(item.Region.HitPoint());
}
