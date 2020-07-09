#pragma once
#include <string>

#include "StringEx.h"
#include "Sqlite.h"

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
		TrafficChannel()
			:ChannelIndex(0), ChannelName(), ChannelUrl(), ChannelType(0), ChannelStatus(0)
			, Loop(true), OutputImage(false), OutputReport(false), OutputRecogn(false), GlobalDetect(false)
		{

		}
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
		//是否循环
		bool Loop;
		//是否输出图片
		bool OutputImage;
		//是否输出检测报告
		bool OutputReport;
		//是否输出识别数据
		bool OutputRecogn;
		//是否全局检测
		bool GlobalDetect;

		/**
		* @brief: 获取通道rtmp地址
		* @return: 通道rtmp地址
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://", ip, ":1935/live/", ChannelIndex);
		}

		/**
		* @brief: 获取通道http-flv地址
		* @return: 通道http-flv地址
		*/
		std::string FlvUrl(const std::string ip) const
		{
			return StringEx::Combine("http://",ip,":1936/live?port=1935&app=live&stream=", ChannelIndex);
		}
	};

	//数据库
	class TrafficData
	{
	public:
		/**
		* @brief: 构造函数
		*/
		TrafficData();

		/**
		* @brief: 析构函数
		*/
		virtual ~TrafficData()= default;

		/**
		* @brief: 初始化数据库名称
		* @param: dbName 数据库名称
		*/
		static void Init(const std::string& dbName);

		/**
		* @brief: 获取最后一个错误信息
		* @return: 最后一个错误信息
		*/
		std::string LastError();

		/**
		* @brief: 获取参数值
		* @param: key 参数键
		* @return: 参数值
		*/
		std::string GetParameter(const std::string& key);

		/**
		* @brief: 获取参数值
		* @param: key 参数键
		* @param: value 参数值
		* @return: 设置结果
		*/
		bool SetParameter(const std::string& key, const std::string& value);
		
		/**
		* @brief: 更新数据库
		*/
		virtual void UpdateDb();

	protected:
		//数据库名称
		static std::string _dbName;

		//数据写入
		SqliteWriter _sqlite;
	};
}



