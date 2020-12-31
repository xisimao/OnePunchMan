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

