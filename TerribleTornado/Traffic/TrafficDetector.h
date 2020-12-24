#pragma once
#include "turbojpeg.h"
#include "opencv2/opencv.hpp"

#include "Shape.h"
#include "MqttChannel.h"
#include "ImageConvert.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//检测元素状态
	enum class DetectStatus
	{
		//在区域外
		Out,
		//首次进入
		New,
		//在区域内
		In
	};

	//检测元素类型
	enum class DetectType
	{
		None = 0,
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
		DetectItem()
			:Id(),Region(),Type(DetectType::None),Status(DetectStatus::Out), Distance(0.0)
		{

		}
		std::string Id;
		//输入
		//检测区域
		Rectangle Region;
		//检测元素类型
		DetectType Type;

		//输出
		//检测元素状态
		DetectStatus Status;
		//车辆距离停止线距离(px)
		double Distance;

		std::string ToJson()
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "id", Id);
			JsonSerialization::SerializeValue(&json, "region", Region.ToJson());
			JsonSerialization::SerializeValue(&json, "type", static_cast<int>(Type));
			return json;
		}
	};

	//识别项
	class RecognItem
	{
	public:
		RecognItem()
			:ChannelIndex(0), FrameIndex(0), FrameSpan(0), TaskId(0),Guid(),Type(DetectType::None),Region(),Width(0),Height(0)
		{

		}
		//通道序号
		int ChannelIndex;
		//帧序号
		int FrameIndex;
		//帧间隔时间(毫秒)
		unsigned char FrameSpan;
		//任务号
		unsigned char TaskId;
		//guid
		std::string Guid;
		//检测项类型
		DetectType Type;
		//检测区域
		Rectangle Region;
		//宽度
		int Width;
		//高度
		int Height;
	};

	//检测类型
	enum class VideoStructType
	{
		Vehicle = 1,
		Bike = 2,
		Pedestrain = 3
	};

	//机动车结构化数据
	class VideoStruct_Vehicle
	{
	public:
		VideoStruct_Vehicle()
			:LaneId(),LaneName(),Direction(0), Minute(0), Second(0), CarType(0),CarColor(0),CarBrand(),PlateType(0),PlateNumber()
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Direction;
		int Minute;
		int Second;
		int CarType;
		int CarColor;
		std::string CarBrand;
		int PlateType;
		std::string PlateNumber;

		/**
		* 获取数据的json格式
		* @return 数据的json格式
		*/
		std::string ToJson()
		{
			std::string vehicleJson;
			JsonSerialization::SerializeValue(&vehicleJson, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&vehicleJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&vehicleJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&vehicleJson, "direction", Direction);
			JsonSerialization::SerializeValue(&vehicleJson, "carType", CarType);
			JsonSerialization::SerializeValue(&vehicleJson, "carColor", CarColor);
			JsonSerialization::SerializeValue(&vehicleJson, "carBrand", CarBrand);
			JsonSerialization::SerializeValue(&vehicleJson, "plateType", PlateType);
			JsonSerialization::SerializeValue(&vehicleJson, "plateNumber", PlateNumber);
			return vehicleJson;
		}
	};

	//非机动车结构化数据
	class VideoStruct_Bike
	{
	public:
		VideoStruct_Bike()
			:LaneId(), LaneName(), Direction(0), Minute(0), Second(0), BikeType(0)
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Direction;
		int Minute;
		int Second;
		int BikeType;

		/**
		* 获取数据的json格式
		* @return 数据的json格式
		*/
		std::string ToJson()
		{
			std::string bikeJson;
			JsonSerialization::SerializeValue(&bikeJson, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&bikeJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&bikeJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&bikeJson, "direction", Direction);
			JsonSerialization::SerializeValue(&bikeJson, "bikeType", BikeType);
			return bikeJson;
		}
	};

	//行人结构化数据
	class VideoStruct_Pedestrain
	{
	public:
		VideoStruct_Pedestrain()
			:LaneId(), LaneName(), Direction(0), Minute(0), Second(0), Sex(0),Age(0),UpperColor(0)
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Direction;
		int Minute;
		int Second;
		int Sex;
		int Age;
		int UpperColor;

		/**
		* 获取数据的json格式
		* @return 数据的json格式
		*/
		std::string ToJson()
		{
			std::string pedestrainJson;
			JsonSerialization::SerializeValue(&pedestrainJson, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&pedestrainJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&pedestrainJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&pedestrainJson, "direction", Direction);
			JsonSerialization::SerializeValue(&pedestrainJson, "sex", Sex);
			JsonSerialization::SerializeValue(&pedestrainJson, "age", Age);
			JsonSerialization::SerializeValue(&pedestrainJson, "upperColor", UpperColor);
			return pedestrainJson;
		}
	};

	//通道检测
	class TrafficDetector
	{
	public:
		/**
		* 构造函数
		* @param width 图片宽度
		* @param height 图片高度
		* @param mqtt mqtt
		*/
		TrafficDetector(int width, int height, MqttChannel* mqtt);

		/**
		* 析构函数
		*/
		virtual ~TrafficDetector() {};

		/**
		* 获取车道是否初始化成功
		* @return 车道是否初始化成功
		*/
		bool LanesInited() const;

	protected:
		//视频序号
		int _channelIndex;
		//视频地址
		std::string _channelUrl;
		//视频宽度
		int _width;
		//视频高度
		int _height;
		//mqtt
		MqttChannel* _mqtt;

		//车道初始化是否成功
		bool _lanesInited;

		//bgr字节流长度
		int _bgrSize;
		//jpg字节流长度
		int _jpgSize;
	};
}

