#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"
#include "FlowChannel.h"

namespace OnePunchMan
{
	//检测元素状态
	enum class DetectStatus
	{
		New,
		In,
		Out
	};

	//检测元素类型
	enum class DetectType
	{
		None=0,
		Pedestrain = 1,
		Bike = 2,
		Motobike = 3,
		Car = 4,
		Tricycle = 5,
		Bus = 6,
		Van = 7,
		Truck = 8
	};

	//检测项
	class DetectItem
	{
	public:
		//输入
		//检测区域
		Rectangle Region;
		//检测元素类型
		DetectType Type;
		
		//输出
		//检测元素状态
		DetectStatus Status;
		//移动距离
		double Distance;
	};
	
	//识别项
	class RecognItem
	{
	public:
		//通道序号
		int ChannelIndex;
		//guid
		std::string Guid;
		//检测项类型
		int Type;
		//检测区域
		Rectangle Region;
		//宽度
		int Width;
		//高度
		int Height;
	};

	//交通状态
	enum class TrafficStatus
	{
		Good = 1,
		Normal = 2,
		Warning = 3,
		Bad = 4,
		Dead = 5
	};

	//io数据项
	class IOItem
	{
	public:
		//车道编号
		std::string LaneId;
		//检测类型
		DetectType Type;
		//io状态
		bool Status;
		//io状态是否改变
		bool Changed;
	};

	//流量数据项
	class FlowItem
	{
	public:
		//车道编号
		std::string LaneId;

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
		//交通状态
		int TrafficStatus;
	};

	//视频结构化
	class VideoStruct
	{
	public:
		std::string Image;
		std::string Feature;
		int VideoStructType;
	};

	//机动车
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

	//非机动车
	class VideoBike :public VideoStruct
	{
	public:
		VideoBike()
		{
			VideoStructType = 2;
		}
		int BikeType;
	};

	//行人
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
		* @param: laneId 车道编号
		* @param: lane 车道
		*/
		LaneDetector(const std::string& laneId,const Lane& lane);

		/**
		* @brief: 检测机动车
		* @param: item 检测数据项
		* @return: 返回车道io状态
		*/
		IOItem Detect(std::map<std::string, DetectItem>* items,long long timeStamp);

		/**
		* @brief: 识别机动车
		* @param: item 识别数据项
		* @return: 如果识别项在车道内返回车道编号否则返回空字符串
		*/
		std::string Recogn(const RecognItem& item);

		/**
		* @brief: 收集车道计算数据
		* @param: item 结算时间戳
		* @return: 车道计算数据
		*/
		FlowItem Collect(long long timeStamp);
		
		/**
		* @brief: 获取当前检测区域
		* @return: 当前检测区域
		*/
		const Polygon& Region();

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
		
		/**
		* @brief: 解析多边形字符串
		* @param: region 区域字符串
		* @return: 多边形
		*/
		Polygon GetPolygon(const std::string& region);
		
		//车道编号
		std::string _laneId;
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

		//用于计算平均速度
		//车辆行驶总距离(像素值) 
		double _totalDistance;
		//车辆行驶总时间(毫秒)
		long long _totalTime;

		//用于计算时间占有率
		//区域占用总时间(毫秒)
		long long _totalInTime;

		//用于计算车头时距
		//上一次有车进入区域的时间戳 
		long long _lastInRegion;
		//机动车总数
		int _vehicles;
		//车辆进入区域时间差的和(毫秒)
		long long _totalSpan;

		//io状态
		bool _iOStatus;

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

