#pragma once
#include <string>
#include <math.h>

#include "StringEx.h"
#include "Sqlite.h"

namespace OnePunchMan
{
	//流量车道
	class FlowLane 
	{
	public:
		FlowLane()
			: ChannelIndex(), LaneIndex(), LaneName(), LaneId(), Direction(0), FlowDirection(0)
			, StopLine(), FlowDetectLine(), QueueDetectLine(), LaneLine1(), LaneLine2(), FlowRegion(),QueueRegion()
			, IOIp(), IOPort(0), IOIndex(0), ReportProperties(0)
		{

		}
		//通道序号
		int ChannelIndex;
		//车道序号
		int LaneIndex;
		//车道名称
		std::string LaneName;
		//车道编号
		std::string LaneId;
		//车道方向
		int Direction;
		//车道流向
		int FlowDirection;
		//停止线
		std::string StopLine;
		//流量检测线
		std::string FlowDetectLine;
		//排队检测线
		std::string QueueDetectLine;
		//车道线1
		std::string LaneLine1;
		//车道线2
		std::string LaneLine2;
		//流量检测区域
		std::string FlowRegion;
		//排队检测区域
		std::string QueueRegion;
		//io检测器地址
		std::string IOIp;
		//io检测器端口
		int IOPort;
		//io检测器输出口
		int IOIndex;
		//上报属性
		int ReportProperties;
	};

	//事件车道类型
	enum class EventLaneType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Lane = 3,
		Bike = 4
	};

	//事件类型
	enum class EventType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Congestion = 3,
		Retrograde = 4,
		Bike = 5
	};

	//事件车道
	class EventLane 
	{
	public:
		EventLane()
			:ChannelIndex(), LaneIndex(), LaneName(), LaneType(0), Line(), Region()
		{

		}
		//通道序号
		int ChannelIndex;
		//车道序号
		int LaneIndex;
		//车道名称
		std::string LaneName;
		//车道类型
		int LaneType;
		//箭头
		std::string Line;
		//区域
		std::string Region;
	};

	//通道类型
	enum class ChannelType
	{
		None = 0,
		GB28181 = 1,
		RTSP = 2,
		File = 3
	};

	//通道状态
	enum class ChannelStatus
	{
		//正常
		Normal = 1,
		//无法打开视频源
		InputError = 2,
		//视频输出错误
		OutputError = 3,
		//解码器异常
		DecoderError = 4,
		//无法读取视频数据
		ReadError = 5,
		//解码错误
		DecodeError = 6,
		//准备循环播放
		ReadEOF_Restart = 7,
		//文件播放结束
		ReadEOF_Stop = 8,
		//正在初始化
		Init = 9,
		//网络异常(中心)
		Disconnect = 10,
		//通道不同步(中心)
		NotFoundChannel = 11,
		//车道不同步(中心)
		NotFoundLane = 12,
		//过多帧没有处理
		NotHandle = 13,
		//过滤avc1格式错误
		FilterError = 14
	};

	//视频通道
	class TrafficChannel
	{
	public:
		TrafficChannel()
			:ChannelIndex(0), ChannelName(), ChannelUrl(), ChannelType(0), DeviceId()
			, Loop(true), OutputDetect(false), OutputImage(false), OutputReport(false), OutputRecogn(false), GlobalDetect(false)
			, LaneWidth(0.0), ReportProperties(3)
			, ChannelStatus(0)
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
		//国标设备编号
		std::string DeviceId;

		//是否循环
		bool Loop;
		//是否输出检测数据
		bool OutputDetect;
		//是否输出图片
		bool OutputImage;
		//是否输出检测报告
		bool OutputReport;
		//是否输出识别数据
		bool OutputRecogn;
		//是否全局检测
		bool GlobalDetect;
	
		//车道宽度
		double LaneWidth;
		//上报属性
		int ReportProperties;

		//流量车道集合
		std::vector<FlowLane> FlowLanes;
		//事件车道集合
		std::vector<EventLane> EventLanes;

		//通道状态
		int ChannelStatus;

		/**
		* 获取通道rtmp地址
		* @return 通道rtmp地址
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://", ip, ":1935/live/", ChannelIndex);
		}

		/**
		* 获取通道http-flv地址
		* @return 通道http-flv地址
		*/
		std::string FlvUrl(const std::string ip) const
		{
			return StringEx::Combine("http://", ip, ":1936/live?port=1935&app=live&stream=", ChannelIndex);
		}
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

	//流量报告数据
	class FlowReportData
	{
	public:
		FlowReportData()
			: ChannelUrl(), LaneId(), LaneName(), Direction(0), ReportProperties(0)
			, Minute(0), TimeStamp(0)
			, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
			, Speed(0.0), TimeOccupancy(0.0), HeadDistance(0.0), HeadSpace(0.0), TrafficStatus(0)
			, QueueLength(0), SpaceOccupancy(0.0)
		{

		}
		//通道地址
		std::string ChannelUrl;
		//车道编号
		std::string LaneId;
		//车道名称
		std::string LaneName;
		//车道方向
		int Direction;
		//需要上报的属性
		int ReportProperties;

		//第几分钟
		int Minute;
		//时间戳
		long long TimeStamp;

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
		//公交车流量
		int Buss;
		//面包车流量
		int Vans;
		//卡车流量
		int Trucks;

		//平均速度(km/h)
		double Speed;
		//时间占用率(%)
		double TimeOccupancy;
		//车道时距(sec)
		double HeadDistance;
		//车头间距(m)
		double HeadSpace;
		//交通状态
		int TrafficStatus;

		//排队长度(m)
		int QueueLength;
		//空间占有率(%)
		double SpaceOccupancy;

		/**
		* 获取excel用的json
		* @return excel用的json
		*/
		std::string ToReportJson()
		{
			std::string reportJson;
			JsonSerialization::SerializeValue(&reportJson, "minute", Minute);
			JsonSerialization::SerializeValue(&reportJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&reportJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&reportJson, "direction", Direction);
			JsonSerialization::SerializeValue(&reportJson, "persons", Persons);
			JsonSerialization::SerializeValue(&reportJson, "bikes", Bikes);
			JsonSerialization::SerializeValue(&reportJson, "motorcycles", Motorcycles);
			JsonSerialization::SerializeValue(&reportJson, "cars", Cars);
			JsonSerialization::SerializeValue(&reportJson, "tricycles", Tricycles);
			JsonSerialization::SerializeValue(&reportJson, "buss", Buss);
			JsonSerialization::SerializeValue(&reportJson, "vans", Vans);
			JsonSerialization::SerializeValue(&reportJson, "trucks", Trucks);
			JsonSerialization::SerializeValue(&reportJson, "averageSpeed", static_cast<int>(Speed));
			JsonSerialization::SerializeValue(&reportJson, "headDistance", HeadDistance);
			JsonSerialization::SerializeValue(&reportJson, "headSpace", HeadSpace);
			JsonSerialization::SerializeValue(&reportJson, "timeOccupancy", static_cast<int>(TimeOccupancy));
			JsonSerialization::SerializeValue(&reportJson, "trafficStatus", TrafficStatus);
			JsonSerialization::SerializeValue(&reportJson, "queueLength", QueueLength);
			JsonSerialization::SerializeValue(&reportJson, "spaceOccupancy", SpaceOccupancy);
			return reportJson;
		}

		/**
		* 获取mqtt报告用的json
		* @return mqtt报告用的json
		*/
		std::string ToMessageJson()
		{
			std::string messageJson;
			JsonSerialization::SerializeValue(&messageJson, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&messageJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&messageJson, "timeStamp", TimeStamp);

			JsonSerialization::SerializeValue(&messageJson, "persons", Persons);
			JsonSerialization::SerializeValue(&messageJson, "bikes", Bikes);
			JsonSerialization::SerializeValue(&messageJson, "motorcycles", Motorcycles);
			JsonSerialization::SerializeValue(&messageJson, "cars", Cars);
			JsonSerialization::SerializeValue(&messageJson, "tricycles", Tricycles);
			JsonSerialization::SerializeValue(&messageJson, "buss", Buss);
			JsonSerialization::SerializeValue(&messageJson, "vans", Vans);
			JsonSerialization::SerializeValue(&messageJson, "trucks", Trucks);

			JsonSerialization::SerializeValue(&messageJson, "averageSpeed", static_cast<int>(Speed));
			JsonSerialization::SerializeValue(&messageJson, "headDistance", StringEx::Rounding(HeadDistance,2));
			JsonSerialization::SerializeValue(&messageJson, "headSpace", StringEx::Rounding(HeadSpace, 2));
			JsonSerialization::SerializeValue(&messageJson, "timeOccupancy", static_cast<int>(TimeOccupancy));
			JsonSerialization::SerializeValue(&messageJson, "trafficStatus", TrafficStatus);

			JsonSerialization::SerializeValue(&messageJson, "queueLength", QueueLength);
			JsonSerialization::SerializeValue(&messageJson, "spaceOccupancy", SpaceOccupancy);
			return messageJson;
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
			:Id(0), DeviceId(), DeviceName(), DeviceIp(), DevicePort(0), UserName(), Password()
		{

		}
		int Id;
		std::string DeviceId;
		std::string DeviceName;
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
			:Id(0), ChannelId(), ChannelName()
		{

		}
		int Id;
		std::string ChannelId;
		std::string ChannelName;
	};

	//配置信息
	class TrafficDirectory
	{
	public:
		//文件临时存放目录
		static std::string TempDir;
		//数据存放目录
		static std::string DataDir;
		//文件存放目录
		static std::string FileDir;
		//文件目录连接
		static std::string FileLink;
		//页面目录
		static std::string WebDir;

		/**
		* 初始化目录配置
		* @param jd json配置
		*/
		static void Init(const std::string& webDir)
		{
			WebDir = webDir;
		}
	};

	//数据库
	class TrafficData
	{
	public:
		/**
		* 构造函数
		* @param dbName 数据库地址
		*/
		TrafficData(const std::string& dbName="traffic.db");

		/**
		* 获取最后一个错误信息
		* @return 最后一个错误信息
		*/
		std::string LastError();

		/**
		* 获取参数值
		* @param key 参数键
		* @return 参数值
		*/
		std::string GetParameter(const std::string& key);

		/**
		* 获取参数值
		* @param key 参数键
		* @param value 参数值
		* @return 设置结果
		*/
		bool SetParameter(const std::string& key, const std::string& value);
		
		/**
		* 设置国标参数值
		* @param parameter 国标参数
		* @return 设置结果
		*/
		bool SetGbPrameter(const GbParameter& parameter);

		/**
		* 获取国标参数
		* @return 国标参数
		*/
		GbParameter GetGbPrameter();

		/**
		* 获取国标设备集合
		* @return 国标设备集合
		*/
		std::vector<GbDevice> GetGbDevices();

		/**
		* 添加国标设备
		* @param device 国标设备
		* @return 添加成功返回主键否则返回-1
		*/
		int InsertGbDevice(const GbDevice& device);

		/**
		* 更新国标设备
		* @param device 国标设备
		* @return 更新结果
		*/
		bool UpdateGbDevice(const GbDevice& device);

		/**
		* 删除国标设备
		* @param deviceId 国标设备编号
		* @return 删除结果
		*/
		bool DeleteGbDevice(int deviceId);

		/**
		* 查询国标通道集合
		* @param deviceId 国标设备编号
		* @return 查询结果
		*/
		std::vector<GbChannel> GetGbChannels(const std::string& deviceId);

		/**
		* 查询通道列表
		* @return 通道列表
		*/
		std::vector<TrafficChannel> GetChannels();

		/**
		* 查询单个通道
		* @param channelIndex 通道序号
		* @return 通道
		*/
		TrafficChannel GetChannel(int channelIndex);

		/**
		* 设置通道
		* @param channel 通道
		* @return 设置结果
		*/
		bool SetChannel(const TrafficChannel& channel);

		/**
		* 设置通道集合
		* @param channels 通道集合
		* @return 设置结果
		*/
		bool SetChannels(const std::vector<TrafficChannel>& channels);

		/**
		* 删除通道
		* @param channel 通道
		* @return 删除结果
		*/
		bool DeleteChannel(int channelIndex);

		/**
		* 清空通道
		*/
		void ClearChannels();

		/**
		* 查询车道列表
		* @param channelIndex 通道序号
		* @param laneId 车道编号
		* @return 车道列表
		*/
		std::vector<FlowLane> GetFlowLanes(int channelIndex, const std::string& laneId);

		/**
		* 更新数据库
		*/
		virtual void UpdateDb();

	private:
		/**
		* 填充通道
		* @param sqlite 查询结果
		* @param channel 要填充的通道数据
		*/
		TrafficChannel FillChannel(const SqliteReader& sqlite);

		/**
		* 填充流量车道
		* @param sqlite 查询结果
		* @return 流量车道
		*/
		FlowLane FillFlowLane(const SqliteReader& sqlite);

		/**
		* 填充事件车道
		* @param sqlite 查询结果
		* @return 事件车道
		*/
		EventLane FillEventLane(const SqliteReader& sqlite);

		/**
		* 添加通道
		* @param channel 通道
		* @return 添加结果
		*/
		bool InsertChannel(const TrafficChannel& channel);

		//数据库地址
		std::string _dbName;

		//数据写入
		SqliteWriter _sqlite;
	};


}



