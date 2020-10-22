#pragma once
#include "turbojpeg.h"
#include "opencv2/opencv.hpp"

#include "Shape.h"
#include "MqttChannel.h"
#include "IVEHandler.h"
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
			:Region(),Type(DetectType::None),Status(DetectStatus::Out), Distance(0.0)
		{

		}
		//输入
		//检测区域
		Rectangle Region;
		//检测元素类型
		DetectType Type;
		/**
		* 获取检测项长度
		* @return 检测项长度
		*/
		int GetLength() const
		{
			switch (Type)
			{
			case DetectType::Car:
			case DetectType::Van:
				return 5;
			case DetectType::Tricycle:
				return 4;
			case DetectType::Bus:
				return 9;
			case DetectType::Truck:
				return 13;
			default:
				return 0;
			}
		}
		//输出
		//检测元素状态
		DetectStatus Status;
		//车辆距离停止线距离(px)
		double Distance;
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
	};

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
	};

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

		/**
		* 将下一帧视频写入到bmp
		*/
		void WriteBmp();

		/**
		* 获取默认检测参数
		* @return 默认检测参数
		*/
		std::string GetDetectParam();

		/**
		* 供子类实现的处理检测数据
		* @param items 检测数据项集合
		* @param timeStamp 时间戳
		* @param param 检测参数
		* @param taskId 任务编号
		* @param iveBuffer 图片字节流
		* @param frameIndex 帧序号
		* @param frameSpan 帧间隔时间(毫秒)
		*/
		virtual void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex, unsigned char frameSpan) = 0;
		
		/**
		* 结束检测
		* @param taskId 任务编号
		*/
		virtual void FinishDetect(unsigned char taskId) {};

		/**
		* 处理机动车识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param vehicle 机动车识别数据
		*/
		virtual void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle) {}
		
		/**
		* 处理非机动车识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param bike 非机动车识别数据
		*/
		virtual void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike) {}
		
		/**
		* 处理行人识别数据
		* @param recognItem 识别数据项
		* @param iveBuffer 图片字节流
		* @param pedestrain 行人识别数据
		*/
		virtual void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain) {}

	protected:
		/**
		* 获取车道检测参数
		* @param regions 车道区域集合,json字符串
		* @return 车道检测参数
		*/
		std::string GetDetectParam(const std::string& regions);

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
		//车道算法筛选区域参数
		std::string _param;
		//是否设置过车道参数
		bool _setParam;
		//是否需要写入下一帧到bmp
		bool _writeBmp;

		//bgr字节流长度
		int _bgrSize;
		//jpg字节流长度
		int _jpgSize;

		//ive写入bmp
		IVEHandler _iveHandler;

	};

}

