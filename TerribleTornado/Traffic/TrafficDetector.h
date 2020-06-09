#pragma once
#include "Shape.h"
#include "MqttChannel.h"
#include "JPGHandler.h"

#include "turbojpeg.h"

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
			:Region(),Type(DetectType::None),Status(DetectStatus::Out)
		{

		}
		//输入
		//检测区域
		Rectangle Region;
		//检测元素类型
		DetectType Type;
		//输出
		//检测元素状态
		DetectStatus Status;
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
			:CarType(0),CarColor(0),CarBrand(),PlateType(0),PlateNumber()
		{

		}

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
			:BikeType(0)
		{

		}
		int BikeType;
	};

	class VideoStruct_Pedestrain
	{
	public:
		VideoStruct_Pedestrain()
			:Sex(0),Age(0),UpperColor(0)
		{

		}

		int Sex;
		int Age;
		int UpperColor;
	};

	//通道检测
	class TrafficDetector
	{
	public:
		/**
		* @brief: 构造函数
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: mqtt mqtt
		* @param: debug 是否处于调试模式，处于调试模式则输出画线后的bmp
		*/
		TrafficDetector(int width, int height, MqttChannel* mqtt, bool debug);

		/**
		* @brief: 析构函数
		*/
		virtual ~TrafficDetector();

		/**
		* @brief: 获取车道是否初始化成功
		* @return 车道是否初始化成功
		*/
		bool LanesInited() const;

		/**
		* @brief: 供子类实现的处理检测数据
		* @param: items 检测数据项集合
		* @param: timeStamp 时间戳
		* @param: param 检测参数
		* @param: iveBuffer 图片字节流
		* @param: frameIndex 帧序号
		* @param: frameSpan 帧间隔时间(毫秒)
		*/
		virtual void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, const unsigned char* yuvBuffer, int frameIndex,int frameSpan) = 0;

		/**
		* @brief: 处理机动车识别数据
		* @param: recognItem 识别数据项
		* @param: iveBuffer 图片字节流
		* @param: vehicle 机动车识别数据
		*/
		virtual void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle) {}
		
		/**
		* @brief: 处理非机动车识别数据
		* @param: recognItem 识别数据项
		* @param: iveBuffer 图片字节流
		* @param: bike 非机动车识别数据
		*/
		virtual void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike) {}
		
		/**
		* @brief: 处理行人识别数据
		* @param: recognItem 识别数据项
		* @param: iveBuffer 图片字节流
		* @param: pedestrain 行人识别数据
		*/
		virtual void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain) {}

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
		//车道算法筛选区域参数
		std::string _param;
		//是否设置过车道参数
		bool _setParam;

		//bgr字节流长度
		int _bgrSize;
		//bgr字节流
		unsigned char* _bgrBuffer;
		//jpg字节流长度
		int _jpgSize;
		//jpg字节流
		unsigned char* _jpgBuffer;
	
		//调试相关
		//是否处于调试模式
		bool _debug;
		//调试时写jpg
		JPGHandler _jpgHandler;

	};

}

