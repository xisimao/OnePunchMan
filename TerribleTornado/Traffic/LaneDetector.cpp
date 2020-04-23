#include "LaneDetector.h"

using namespace std;
using namespace Saitama;
using namespace TerribleTornado;

LaneDetector::LaneDetector(const string& laneId,const Lane& lane)
	: _laneId(laneId),_persons(0),_bikes(0), _motorcycles(0), _cars(0),_tricycles(0), _buss(0),_vans(0),_trucks(0)
	, _totalDistance(0.0), _totalTime(0)
	, _totalInTime(0)
	,_lastInRegion(0), _vehicles(0), _totalSpan(0), _iOStatus(IOStatus::Out)
	,_lastTimeStamp(0),_currentItems(&_items1),_lastItems(&_items2)
{
	InitLane(lane);
}

void LaneDetector::InitLane(const Lane& lane)
{
	Line detectLine = GetLine(lane.DetectLine);
	Line stopLine = GetLine(lane.StopLine);
	Line laneLine1 = GetLine(lane.LaneLine1);
	Line laneLine2 = GetLine(lane.LaneLine2);
	if (detectLine.Empty() ||
		stopLine.Empty() ||
		laneLine1.Empty() ||
		laneLine2.Empty())
	{
		_region = Polygon();
		_meterPerPixel = 0;
		LogPool::Warning(LogEvent::Detect,"line empty");
		return;
	}
	else
	{
		Point point1 = detectLine.Intersect(laneLine1);
		Point point2 = detectLine.Intersect(laneLine2);
		Point point3 = stopLine.Intersect(laneLine1);
		Point point4 = stopLine.Intersect(laneLine2);
		if (point1.Empty() ||
			point2.Empty() ||
			point3.Empty() ||
			point4.Empty())
		{
			_region = Polygon();
			_meterPerPixel = 0;
			LogPool::Warning(LogEvent::Detect,"intersect point empty");
			return;
		}
		vector<Point> points;
		points.push_back(point1);
		points.push_back(point2);
		points.push_back(point3);
		points.push_back(point4);
		//_region= Polygon(points);
		_region = GetPolygon(lane.Region);
		Line line1(point1, point2);
		Line line2(point3, point4);
		double pixels = line1.Middle().Distance(line2.Middle());
		_meterPerPixel = lane.Length / pixels;
	}
}

Line LaneDetector::GetLine(const string& line)
{
	if (line.size() > 2)
	{
		vector<Point> points;
		vector<string> coordinates = StringEx::Split(line.substr(1, line.size() - 2), ",", true);
		int x, y = 0;
		for (unsigned int i = 0; i < coordinates.size(); ++i)
		{
			if (coordinates[i].size() > 2)
			{
				if (i % 2 == 0)
				{
					x = StringEx::Convert<int>(coordinates[i].substr(1, coordinates[i].size() - 1));
				}
				else
				{
					y = StringEx::Convert<int>(coordinates[i].substr(0, coordinates[i].size() - 1));
					points.push_back(Point(x, y));
				}
			}
		}
		if (points.size() >= 2)
		{
			return Line(points[0], points[1]);
		}
	}
	return Line();
}

Polygon LaneDetector::GetPolygon(const std::string& region)
{
	vector<Point> points;
	if (region.size() > 2)
	{
		vector<string> coordinates = StringEx::Split(region.substr(1, region.size() - 2), ",", true);
		int x, y = 0;
		for (unsigned int i = 0; i < coordinates.size(); ++i)
		{
			if (coordinates[i].size() > 2)
			{
				if (i % 2 == 0)
				{
					x = StringEx::Convert<int>(coordinates[i].substr(1, coordinates[i].size() - 1));
				}
				else
				{
					y = StringEx::Convert<int>(coordinates[i].substr(0, coordinates[i].size() - 1));
					points.push_back(Point(x, y));
				}
			}
		}
	
	}
	return Polygon(points);
}

IOItem LaneDetector::Detect(const map<string, DetectItem>& items,long long timeStamp)
{
	lock_guard<mutex> lck(_mutex);
	_currentItems->clear();
	IOItem item;
	item.LaneId = _laneId;
	item.Status= IOStatus::Out;
	item.Type = 0;
	for (map<string, DetectItem>::const_iterator it = items.begin(); it!=items.end();++it)
	{
		if (_region.Contains(it->second.HitPoint))
		{
			item.Status = IOStatus::In;
			item.Type = it->second.Type;
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
			}
			//如果是已经记录的车则计算平均速度
			//平均速度=总距离/总时间
			//总距离=两次检测到的点的距离*每个像素代表的米数
			//总时间=两次检测到的时间戳时长
			else
			{
				_totalDistance += it->second.HitPoint.Distance(mit->second.HitPoint);
				_totalTime += timeStamp - _lastTimeStamp;
			}
			_currentItems->insert(pair<string, DetectItem>(it->first, it->second));
		}
	}

	//如果上一次有车，则认为到这次检测为止都有车
	//时间占有率=总时间/一分钟
	if (!_lastItems->empty())
	{
		_totalInTime += timeStamp - _lastTimeStamp;
	}

	map<string, DetectItem>* temp = _currentItems;
	_currentItems = _lastItems;
	_lastItems = temp;

	_lastTimeStamp = timeStamp;

	if (item.Status != _iOStatus)
	{
		_iOStatus = item.Status;
		LogPool::Debug(LogEvent::Detect, "lane:", _laneId, "io:", (int)item.Status);
		return item;
	}
	else
	{
		item.Status = IOStatus::UnChanged;
		return item;
	}
}

string LaneDetector::Recogn(const RecognItem& item)
{
	return _region.Contains(item.HitPoint) ? _laneId : string();
}

FlowItem LaneDetector::Collect(long long timeStamp)
{
	lock_guard<mutex> lck(_mutex);
	FlowItem item;
	item.LaneId = _laneId;

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
	item.TimeOccupancy = static_cast<double>(_totalInTime) / 60000.0 * 100;

	if (item.Speed > 40)
	{
		item.TrafficStatus = static_cast<int>(TrafficStatus::Good);
	}
	else if (item.Speed <= 40 && item.Speed > 30)
	{
		item.TrafficStatus = static_cast<int>(TrafficStatus::Normal);
	}
	else if (item.Speed <= 30 && item.Speed > 20)
	{
		item.TrafficStatus = static_cast<int>(TrafficStatus::Warning);
	}
	else if (item.Speed <= 20 && item.Speed > 15)
	{
		item.TrafficStatus = static_cast<int>(TrafficStatus::Warning);
	}
	else
	{
		item.TrafficStatus = static_cast<int>(TrafficStatus::Dead);
	}
	
	LogPool::Debug(LogEvent::Detect, "lane:", _laneId, "vehicles:", item.Cars + item.Tricycles + item.Buss + item.Vans + item.Trucks, "bikes:", item.Bikes + item.Motorcycles, "persons:", item.Persons, item.Speed, "km/h ", item.HeadDistance, "sec ", item.TimeOccupancy, "%");

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

	_totalInTime = 0;

	_lastInRegion = 0;
	_vehicles = 0;
	_totalSpan = 0;

	_lastTimeStamp = timeStamp;
	return item;
}


