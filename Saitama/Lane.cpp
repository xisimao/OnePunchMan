#include "Lane.h"

using namespace std;
using namespace Saitama;

Lane::Lane()
	:Lane(Polygon())
{

}

Lane::Lane(Polygon region)
	:_region(region), _persons(0),_bikes(0), _motorcycles(0), _cars(0),_tricycles(0), _buss(0),_vans(0),_trucks(0),  _totalDistance(0.0), _totalTime(0.0), _lastInRegion(0), _vehicles(0), _totalSpan(0.0)
{

}

void Lane::CollectVehicle(const Rectangle& detectRegion, long long timeStamp, const string& message)
{
	if (_region.Contains(detectRegion.HitPoint()))
	{
		string id;
		JsonFormatter::Deserialize(message, tuple<string, string*>("GUID", &id));
		DetectItem item(id, timeStamp, detectRegion);

		std::lock_guard<std::mutex> lck(_mutex);
		map<string, DetectItem>::iterator mit = _items.find(item.Id);

		//如果是新车则计算流量和车头时距
		//流量=车数量
		//车头时距=所有进入区域的时间差的平均值
		if (mit == _items.end())
		{
			int type = 0;
			JsonFormatter::Deserialize(message, tuple<string, int*>("Type", &type));
			if (type == (int)DetectType::Car)
			{
				_cars += 1;
			}
			else if (type == (int)DetectType::Tricycle)
			{
				_tricycles += 1;
			}
			else if (type == (int)DetectType::Bus)
			{
				_buss += 1;
			}
			else if (type == (int)DetectType::Van)
			{
				_vans += 1;
			}
			else if (type == (int)DetectType::Truck)
			{
				_trucks += 1;
			}
			_vehicles += 1;

			if (_lastInRegion != 0)
			{
				_totalSpan += timeStamp - _lastInRegion;
			}
			_lastInRegion = timeStamp;

		}
		//如果是已经记录的车则计算平均速度和时间占有率
		//平均速度=总距离/总时间
		//总距离=两次检测到的点的距离*每个像素代表的米数
		//总时间=两次检测到的时间戳时长
		//时间占有率=总时间/一分钟
		else
		{
			//LogPool::Debug(item.Region.Top().Distance(it->second.Region.Top()) * per, "m ", (static_cast<double>(item.TimeStamp) - static_cast<double>(it->second.TimeStamp)) / 1000.0, "sec ",distance/ time,"km/h");
			_totalDistance += item.Region.HitPoint().Distance(mit->second.Region.HitPoint());
			_totalTime += item.TimeStamp - mit->second.TimeStamp;
		}
		_items[item.Id] = item;
	}
}

void Lane::CollectBike(const Rectangle& detectRegion, long long timeStamp, const string& message)
{
	if (_region.Contains(detectRegion.HitPoint()))
	{
		string id;
		JsonFormatter::Deserialize(message, tuple<string, string*>("GUID", &id));
		DetectItem item(id, timeStamp, detectRegion);

		std::lock_guard<std::mutex> lck(_mutex);
		map<string, DetectItem>::iterator mit = _items.find(item.Id);
		if (mit == _items.end())
		{
			int type = 0;
			JsonFormatter::Deserialize(message, tuple<string, int*>("Type", &type));
			if (type == (int)DetectType::Bike)
			{
				_bikes += 1;
			}
			else if (type == (int)DetectType::Motobike)
			{
				_motorcycles += 1;
			}
		}
		_items[item.Id] = item;
	}
}

void Lane::CollectPedestrain(const Rectangle& detectRegion, long long timeStamp, const string& message)
{
	if (_region.Contains(detectRegion.HitPoint()))
	{
		string id;
		JsonFormatter::Deserialize(message, tuple<string, string*>("GUID", &id));
		DetectItem item(id, timeStamp, detectRegion);

		std::lock_guard<std::mutex> lck(_mutex);
		map<string, DetectItem>::iterator mit = _items.find(item.Id);
		if (mit == _items.end())
		{
			_persons += 1;
		}
		_items[item.Id] = item;
	}
}

TrafficItem Lane::Calculate()
{
	
	std::lock_guard<std::mutex> lck(_mutex);
	TrafficItem item;
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
	item.HeadDistance = _totalSpan / (static_cast<long long>(_vehicles) - 1) / 1000.0;
	item.TimeOccupancy = _totalTime / 60000.0 * 100;

	LogPool::Information("cars:", _vehicles, "bikes:", _bikes + _motorcycles, "persons:", _persons, item.Speed, "km/h ", item.HeadDistance, "sec ", item.TimeOccupancy, "%");

	_persons = 0;
	_bikes = 0;
	_motorcycles = 0;
	_tricycles = 0;
	_trucks = 0;
	_vans = 0;
	_cars = 0;
	_buss = 0;

	_totalDistance = 0.0;
	_totalTime = 0.0;

	_lastInRegion = 0;
	_vehicles = 0;
	_totalSpan = 0;

	_items.clear();
	return item;
}



