#include "LaneDetector.h"

using namespace std;
using namespace Saitama;

LaneDetector::LaneDetector(const string& id,int index,Polygon region, double meterPerPixel)
	:_id(id),_index(index),_region(region), _persons(0),_bikes(0), _motorcycles(0), _cars(0),_tricycles(0), _buss(0),_vans(0),_trucks(0), _totalDistance(0.0), _totalTime(0), _lastInRegion(0), _vehicles(0), _totalSpan(0), _iOStatus(IOStatus::Out),_lastTimeStamp(0),_meterPerPixel(meterPerPixel),_currentItems(&_items1),_lastItems(&_items2)
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

IOStatus LaneDetector::Detect(const map<string, DetectItem>& items,long long timeStamp)
{
	std::lock_guard<std::mutex> lck(_mutex);
	_currentItems->clear();
	IOStatus status = IOStatus::Out;
	for (map<string, DetectItem>::const_iterator it = items.begin(); it!=items.end();++it)
	{
		if (Contains(it->second))
		{
			status = IOStatus::In;
			map<string, DetectItem>::const_iterator mit = _lastItems->find(it->first);
			//如果是新车则计算流量和车头时距
			//流量=车数量
			//车头时距=所有进入区域的时间差的平均值
			if (mit == _lastItems->end())
			{
				if (it->second.Type == (int)DetectType::Car)
				{
					_cars += 1;
					_vehicles += 1;
				}
				else if (it->second.Type == (int)DetectType::Tricycle)
				{
					_tricycles += 1;
					_vehicles += 1;
				}
				else if (it->second.Type == (int)DetectType::Bus)
				{
					_buss += 1;
					_vehicles += 1;
				}
				else if (it->second.Type == (int)DetectType::Van)
				{
					_vans += 1;
					_vehicles += 1;
				}
				else if (it->second.Type == (int)DetectType::Truck)
				{
					_trucks += 1;
					_vehicles += 1;
				}
				else if (it->second.Type == (int)DetectType::Bike)
				{
					_bikes += 1;
				}
				else if (it->second.Type == (int)DetectType::Motobike)
				{
					_motorcycles += 1;
				}
				else if (it->second.Type == (int)DetectType::Pedestrain)
				{
					_persons += 1;
				}
				if (_lastInRegion != 0)
				{
					_totalSpan += timeStamp - _lastInRegion;
				}
				_lastInRegion = timeStamp;
				LogPool::Debug("i:", it->first," ",it->second.Type);
			}
			//如果是已经记录的车则计算平均速度和时间占有率
			//平均速度=总距离/总时间
			//总距离=两次检测到的点的距离*每个像素代表的米数
			//总时间=两次检测到的时间戳时长
			//时间占有率=总时间/一分钟
			else
			{
				_totalDistance += it->second.HitPoint.Distance(mit->second.HitPoint);
				_totalTime += timeStamp - _lastTimeStamp;

				LogPool::Debug("d:", _totalDistance, " t:", _totalTime);
			}
			_currentItems->insert(pair<string, DetectItem>(it->first, it->second));
		}
	}

	std::map<std::string, DetectItem>* temp = _currentItems;
	_currentItems = _lastItems;
	_lastItems = temp;

	_lastTimeStamp = timeStamp;

	if (status != _iOStatus)
	{
		_iOStatus = status;
		return _iOStatus;
	}
	else
	{
		return IOStatus::UnChanged;
	}
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

	item.Speed = _totalTime==0?0:(_totalDistance * _meterPerPixel / 1000.0) / (static_cast<double>(_totalTime) / 3600000.0);
	item.HeadDistance = _vehicles > 1 ? static_cast<double>(_totalSpan) / static_cast<double>(_vehicles- 1) / 1000.0 : 0;
	item.HeadSpace = item.Speed * 1000 * item.HeadDistance / 3600.0;
	item.TimeOccupancy = static_cast<double>(_totalTime) / 60000.0 * 100;

	LogPool::Debug("lane:", _id, "vehicles:", item.Cars + item.Tricycles + item.Buss + item.Vans + item.Trucks, "bikes:", item.Bikes + item.Motorcycles, "persons:", item.Persons, item.Speed, "km/h ", item.HeadDistance, "sec ", item.TimeOccupancy, "%");

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

bool LaneDetector::Contains(const DetectItem& item)
{
	return _region.Contains(item.HitPoint);
}
