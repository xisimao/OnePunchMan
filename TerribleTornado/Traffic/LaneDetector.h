#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"
#include "FlowChannelData.h"

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
			:DetectItem(Rectangle(), 0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: region 检测元素区域
		*/
		DetectItem(const Rectangle& region)
			:DetectItem(region,0)
		{

		}

		/**
		* @brief: 构造函数
		* @param: region 检测元素区域
		* @param: type 检测元素类型
		*/
		DetectItem(const Rectangle& region,int type)
			:HitPoint(region.Top.X + region.Width / 2, region.Top.Y + region.Height), Type(type)
		{

		}

		//检测点
		Point HitPoint;
		//检测元素类型
		int Type;
		
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

	//io状态
	enum class IOStatus
	{
		In = 1,
		Out = 0,
		UnChanged=-1
	};

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
		//车头间距(米)=平均速度*车头时距
		double HeadSpace;
		//时间占有率(%)
		double TimeOccupancy;
	};

	//视频结构化
	class VideoStruct
	{
	public:
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
		* @param: dataId 数据编号
		* @param: lane 车道
		*/
		LaneDetector(const std::string& dataId,const Lane& lane);

		/**
		* @brief: 获取车道数据编号
		* @return: 车道数据编号io状态
		*/
		std::string DataId();

		/**
		* @brief: 检测机动车
		* @param: item 检测项
		* @return: 返回车道io状态
		*/
		IOStatus Detect(const std::map<std::string, DetectItem>& items,long long timeStamp);

		/**
		* @brief: 检测数据是否在车道范围内
		* @param: item 检测项
		* @return: 在车道内返回true
		*/
		bool Contains(const DetectItem& item);

		/**
		* @brief: 收集车道计算数据
		* @param: item 结算时间戳
		* @return: 车道计算数据
		*/
		LaneItem Collect(long long timeStamp);

	private:

		/**
		* @brief: 初始化车道
		* @param: lane 车道
		*/
		void InitLane(const Lane& lane);

		/**
		* @brief: 解析线段字符串
		* @param: lane 车道线字符串
		* @return: 线段
		*/
		Line GetLine(const std::string& line);

		//数据编号
		std::string _dataId;
		//当前检测区域
		Polygon _region;
		//每个像素代表的米数
		double _meterPerPixel;

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

		//io状态
		IOStatus _iOStatus;

		//上一帧的时间戳
		long long _lastTimeStamp;
		//同步锁
		std::mutex _mutex;
		std::map<std::string, DetectItem> _items1;
		std::map<std::string, DetectItem> _items2;
		std::map<std::string, DetectItem>* _currentItems;
		std::map<std::string, DetectItem>* _lastItems;

	};

}

