#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"

namespace Saitama
{
	class DetectItem
	{
	public:
		DetectItem()
			:DetectItem(std::string(),0,Rectangle())
		{

		}

		DetectItem(const std::string& id,int timeStamp,const Rectangle& region)
			:Id(id), TimeStamp(timeStamp), Region(region)
		{

		}

		std::string Id;
		long long TimeStamp;
		Rectangle Region;
	};

	enum class DetectType
	{
		Pedestrain=1,
		Bike =2,
		Motobike =3,
		Car =4,
		Tricycle =5,
		Bus =6,
		Van =7,
		Truck =8
	};

	class TrafficItem
	{
	public:

		int Bikes;
		int Tricycles;
		int Persons;
		int Cars;
		int Motorcycles;
		int Buss;
		int Trucks;
		int Vans;
		double Speed;
		double HeadDistance;
		double TimeOccupancy;
	};

	class Lane
	{
	public:

		Lane();

		Lane(Polygon region);

		void CollectVehicle(const Rectangle& detectRegion, long long timeStamp, const std::string& message);

		void CollectBike(const Rectangle& detectRegion, long long timeStamp, const std::string& message);

		void CollectPedestrain(const Rectangle& detectRegion,long long timeStamp,const std::string& message);

		TrafficItem Calculate();

	private:

		//同步锁
		std::mutex _mutex;
		//当前检测结果集合
		std::map<std::string, DetectItem> _items;
		//当前检测区域
		Polygon _region;

		//行人流量
		int _persons;
		//自行车流量
		int _bikes;
		//摩托车流量
		int _motorcycles;
		//轿车流量
		int _cars;
		//三轮车流量
		int _tricycles;
		//公交车流量
		int _buss;
		//面包车流量
		int _vans;
		//卡车流量
		int _trucks;

		//总距离(像素值)
		double _totalDistance;
		//总时间(毫秒)
		long long _totalTime;

		//上一次有车进入区域的时间戳
		long long _lastInRegion;
		//机动车总数
		int _vehicles;
		//车辆进入区域时间差的和(毫秒)
		long long _totalSpan;
	};

}

