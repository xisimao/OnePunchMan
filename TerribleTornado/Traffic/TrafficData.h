#pragma once
#include <string>

#include "StringEx.h"
#include "Sqlite.h"

namespace OnePunchMan
{
	//通道类型
	enum class ChannelType
	{
		None=0,
		GB28181 = 1,
		RTSP = 2,
		File = 3
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

	//国标参数
	class GbParameter
	{
	public:
		GbParameter()
			:ServerIp(), ServerPort(0), SipPort(0)
			, SipType(0), GbId(), DomainId(), UserName(), Password()
		{

		}
		std::string ServerIp;
		int ServerPort;
		int SipPort;
		int SipType;
		std::string GbId;
		std::string DomainId;
		std::string UserName;
		std::string Password;
		
	};

	//国标设备
	class GbDevice
	{
	public:
		GbDevice()
			:DeviceId(0),DeviceName(),GbId(),DeviceIp(),DevicePort(0),UserName(),Password()
		{

		}
		int DeviceId;
		std::string DeviceName;
		std::string GbId;
		std::string DeviceIp;
		int DevicePort;
		std::string UserName;
		std::string Password;
	};

	//国标通道
	class GbChannel
	{
	public:
		GbChannel()
			:Id(0),ChannelId(),ChannelName(),DeviceId()
		{

		}
		int Id;
		std::string ChannelId;
		std::string ChannelName;
		std::string DeviceId;
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
		* @brief: 设置国标参数值
		* @param: parameter 国标参数
		* @return: 设置结果
		*/
		bool SetGbPrameter(const GbParameter& parameter);

		/**
		* @brief: 获取国标参数
		* @return: 国标参数
		*/
		GbParameter GetGbPrameter();

		/**
		* @brief: 获取国标设备集合
		* @return: 国标设备集合
		*/
		std::vector<GbDevice> GetGbDeviceList();

		/**
		* @brief: 添加国标设备
		* @param: device 国标设备
		* @return: 添加成功返回主键否则返回-1
		*/
		int InsertGbDevice(const GbDevice& device);

		/**
		* @brief: 更新国标设备
		* @param: device 国标设备
		* @return: 更新结果
		*/
		bool UpdateGbDevice(const GbDevice& device);

		/**
		* @brief: 删除国标设备
		* @param: deviceId 国标设备编号
		* @return: 删除结果
		*/
		bool DeleteGbDevice(int deviceId);

		std::vector<GbChannel> GetGbChannelList(const std::string& gbId);

		/**
		* @brief: 更新数据库
		*/
		virtual void UpdateDb();

	protected:
		/**
		* @brief: 填充通道
		* @param: sqlite 查询结果
		* @param: channel 要填充的通道数据
		*/
		void FillChannel(const SqliteReader& sqlite, TrafficChannel* channel);

		/**
		* @brief: 获取查询集合sql语句
		* @return: 查询集合sql语句
		*/
		std::string GetChannelList();

		/**
		* @brief: 获取查询单个sql语句
		* @param: channelIndex 通道序号
		* @return: 查询单个sql语句
		*/
		std::string GetChannel(int channelIndex);

		/**
		* @brief: 获取添加通道sql语句
		* @param: channel 要添加的通道数据
		* @return: 添加通道sql语句
		*/
		std::string InsertChannel(const TrafficChannel* channel);

		/**
		* @brief: 获取删除通道sql语句
		* @param: channelIndex 通道序号
		* @return: 删除通道sql语句
		*/
		std::string DeleteChannel(int channelIndex);

		/**
		* @brief: 获取清空通道sql语句
		* @return: 清空通道sql语句
		*/
		std::string ClearChannel();

		//数据库名称
		static std::string _dbName;

		//数据写入
		SqliteWriter _sqlite;
	};


}



