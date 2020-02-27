#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "Detect.h"
#include "LogPool.h"
#include "JsonFormatter.h"

namespace Saitama
{
	//车道计算数据项
	class LaneItem
	{
	public:

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

	//车道数据计算
	class Lane
	{
	public:

		/**
		* @brief: 构造函数
		* @param: id 车道编号
		* @param: region 车道区域
		*/
		Lane(const std::string& id,Polygon region);

		/**
		* @brief: 获取车道编号
		* @return: 车道编号
		*/
		const std::string& Id() const;

		/**
		* @brief: 推送机动车检测数据
		* @param: detectRegion 检测区域
		* @param: timeStamp 时间戳
		* @param: data 车道数据
		*/
		void PushVehicle(const Rectangle& detectRegion, long long timeStamp, const std::string& data);

		/**
		* @brief: 推送非机动车检测数据
		* @param: detectRegion 检测区域
		* @param: timeStamp 时间戳
		* @param: data 车道数据
		*/
		void PushBike(const Rectangle& detectRegion, long long timeStamp, const std::string& data);

		/**
		* @brief: 推送行人检测数据
		* @param: detectRegion 检测区域
		* @param: timeStamp 时间戳
		* @param: data 车道数据
		*/
		void PushPedestrain(const Rectangle& detectRegion,long long timeStamp,const std::string& data);

		/**
		* @brief: 收集车道计算数据
		* @return: 车道计算数据
		*/
		LaneItem Collect();

	private:

		//车道编号
		std::string _id;

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

