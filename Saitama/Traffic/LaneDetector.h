#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"

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
			:DetectItem(Rectangle(),std::string(), 0, 0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: region 检测元素区域
		*/
		DetectItem(const Rectangle& region)
			:DetectItem(region,std::string(),0,0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: region 检测元素区域
		* @param: id 检测元素编号
		* @param: timeStamp 时间戳
		* @param: type 检测元素类型
		*/
		DetectItem(const Rectangle& region,const std::string& id, long long timeStamp, int type)
			:Region(region), Id(id), TimeStamp(timeStamp), Type(type)
		{

		}

		//检测元素编号
		std::string Id;
		//检测元素类型
		int Type;
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

	//车道计算数据项
	class LaneItem
	{
	public:

		//车道编号
		std::string Id;
		//车道序号
		int Index;

		//行人流量
		int Persons;
		//自行车流量
		int Bikes;
		//摩托车流量
		int Motorcycles;
		//轿车流量
		int Cars;	
		//三轮车流量
		int Tricycles;
		//客车流量
		int Buss;
		//面包车流量
		int Vans;
		//卡车流量
		int Trucks;
	
		//平均速度(千米/小时)
		double Speed;
		//车头时距(秒)
		double HeadDistance;
		//时间占有率(%)
		double TimeOccupancy;
	};

	class VideoStruct
	{
	public:
		std::string ChannelId;
		std::string LaneId;
		std::string Image;
		std::string Feature;
		int VideoStructType;
	};

	class VideoVehicle :public VideoStruct
	{
	public:

		VideoVehicle()
		{
			VideoStructType = 1;
		}
		int CarType;
		int CarColor;
		std::string CarBrand;
		int PlateType;
		std::string PlateNumber;
	};

	class VideoBike :public VideoStruct
	{
	public:
		VideoBike()
		{
			VideoStructType = 2;
		}
		int BikeType;
	};

	class VideoPedestrain :public VideoStruct
	{
	public:
		VideoPedestrain()
		{
			VideoStructType = 3;
		}
		int Sex;
		int Age;
		int UpperColor;
	};

	//车道数据计算
	class LaneDetector
	{
	public:

		/**
		* @brief: 构造函数
		* @param: id 车道编号
		* @param: index 车道序号
		* @param: region 车道区域
		*/
		LaneDetector(const std::string& id,int index,Polygon region);
	
		/**
		* @brief: 获取车道编号
		* @return: 车道编号
		*/
		std::string Id();

		/**
		* @brief: 获取车道序号
		* @return: 车道序号
		*/
		int Index();

		/**
		* @brief: 检测机动车
		* @param: item 检测项
		* @return: 如果检测项在车道内返回true，否则返回false
		*/
		bool DetectVehicle(const DetectItem& item);

		/**
		* @brief: 检测非机动车
		* @param: item 检测项
		* @return: 如果检测项在车道内返回true，否则返回false
		*/
		bool DetectBike(const DetectItem& item);

		/**
		* @brief: 检测行人
		* @param: item 检测项
		* @return: 如果检测项在车道内返回true，否则返回false
		*/
		bool DetectPedestrain(const DetectItem& item);

		/**
		* @brief: 检测数据是否在车道范围内
		* @param: item 检测项
		* @return: 在车道内返回true
		*/
		bool Contains(const DetectItem& item);

		/**
		* @brief: 收集车道计算数据
		* @return: 车道计算数据
		*/
		LaneItem Collect();

		//车道IO状态
		bool Status;

	private:

		//同步锁
		std::mutex _mutex;
		//当前检测结果集合
		std::map<std::string, DetectItem> _items;

		//车道编号
		std::string _id;
		//车道序号
		int _index;
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

