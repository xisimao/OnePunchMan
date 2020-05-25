#pragma once
#include <string>

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
		//车道序号
		int LaneIndex;
		//车道名称
		std::string LaneName;
		//区域
		std::string Region;
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

}



