#pragma once
#include <string>

#include "Shape.h"

namespace OnePunchMan
{
	//通道类型
	enum class ChannelType
	{
		GB28181 = 1,
		RTSP = 2,
		File = 3,
		ONVIF = 4
	};

	//设备状态
	enum class DeviceStatus
	{
		Normal = 1,
		Error = 2
	};

	//车道
	class TrafficLane
	{
	public:
		//通道序号
		int ChannelIndex;
		//车道编号
		std::string LaneId;
		//车道名称
		std::string LaneName;
		//车道序号
		int LaneIndex;
		//车道方向
		int Direction;
	};

	//视频通道
	class TrafficChannel
	{
	public:
		//通道序号
		int ChannelIndex;
		//通道名称
		std::string ChannelName;
		//通道地址
		std::string ChannelUrl;
		//通道类型
		int ChannelType;
		//通道状态
		int ChannelStatus;

		/**
		* @brief: 获取通道rtmp地址
		* @return: 通道rtmp地址
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://", ip, ":1935/live/", ChannelIndex);
		}
	};

	//检测元素状态
	enum class DetectStatus
	{
		//首次进入
		New,
		//在区域内
		In,
		//在区域外
		Out
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
}



