#pragma once
#include <string>
#include <math.h>

#include "StringEx.h"
#include "Sqlite.h"
#include "Shape.h"

namespace OnePunchMan
{
	//������Ϣ
	class TrafficDirectory
	{
	public:
		//�ļ���ʱ���Ŀ¼
		const static std::string TempDir;
		//���ݴ��Ŀ¼
		const static std::string DataDir;
		//�ļ����Ŀ¼
		const static std::string FileDir;
		//�ļ�Ŀ¼����
		const static std::string FileLink;
		//ҳ��Ŀ¼
		const static std::string WebConfigPath;

		static std::string GetDataFrameJpg(int channelIndex, int frameIndex)
		{
			return StringEx::Combine(TrafficDirectory::DataDir, "images/", channelIndex, "_", frameIndex, ".jpg");
		}

		static std::string GetTempFrameJpg(int channelIndex, int frameIndex)
		{
			return 	StringEx::Combine(TrafficDirectory::TempDir, channelIndex, "_", frameIndex, ".jpg");
		}

		static std::string GetAllTempFrameJpg(int channelIndex)
		{
			return 	StringEx::Combine(TrafficDirectory::TempDir, channelIndex, "_*.jpg");
		}

		static std::string GetChannelJpg(int channelIndex)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, "channel_", channelIndex, ".jpg");
		}

		static std::string GetRecognBmp(const std::string& id)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, "recogn_", id, ".bmp");
		}

		static std::string GetRecognUrl(const std::string ip, const std::string& id)
		{
			return StringEx::Combine("http://", ip, "/", TrafficDirectory::FileLink, "/recogn_", id, ".bmp");
		}

		static std::string GetEventJpg(const std::string& id, int imageIndex)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, "event_", id, "_", imageIndex, ".jpg");
		}

		static std::string GetEventMp4(const std::string& id)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, "event_", id, ".mp4");
		}

		static std::string GetImageLink(const std::string& ip,const std::string& id, int imageIndex)
		{
			return StringEx::Combine("http://", ip, "/", TrafficDirectory::FileLink, "/event_", id, "_", imageIndex, ".jpg");
		}

		static std::string GetVideoLink(const std::string& ip, const std::string& id)
		{
			return StringEx::Combine("http://", ip, "/", TrafficDirectory::FileLink, "/event_", id, ".mp4");
		}
	};

	//��������
	class FlowLane
	{
	public:
		FlowLane()
			: ChannelIndex(), LaneIndex(), LaneName(), LaneId(), Direction(0), FlowDirection(0)
			, StopLine(), FlowDetectLine(), QueueDetectLine(), LaneLine1(), LaneLine2(), FlowRegion(), QueueRegion()
			, IOIp(), IOPort(0), IOIndex(0)
			, ReportProperties(0)
		{

		}
		//ͨ�����
		int ChannelIndex;
		//�������
		int LaneIndex;
		//��������
		std::string LaneName;
		//�������
		std::string LaneId;
		//��������
		int Direction;
		//��������
		int FlowDirection;
		//ֹͣ��
		std::string StopLine;
		//���������
		std::string FlowDetectLine;
		//�ŶӼ����
		std::string QueueDetectLine;
		//������1
		std::string LaneLine1;
		//������2
		std::string LaneLine2;
		//�����������
		std::string FlowRegion;
		//�ŶӼ������
		std::string QueueRegion;
		//io�������ַ
		std::string IOIp;
		//io������˿�
		int IOPort;
		//io����������
		int IOIndex;
		//�ϱ�����
		int ReportProperties;

		std::string ToJson() const
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelIndex", ChannelIndex);
			JsonSerialization::SerializeValue(&json, "laneId", LaneId);
			JsonSerialization::SerializeValue(&json, "laneName", LaneName);
			JsonSerialization::SerializeValue(&json, "laneIndex", LaneIndex);
			JsonSerialization::SerializeValue(&json, "direction", Direction);
			JsonSerialization::SerializeValue(&json, "flowDirection", FlowDirection);
			JsonSerialization::SerializeValue(&json, "ioIp", IOIp);
			JsonSerialization::SerializeValue(&json, "ioPort", IOPort);
			JsonSerialization::SerializeValue(&json, "ioIndex", IOIndex);
			JsonSerialization::SerializeValue(&json, "stopLine", StopLine);
			JsonSerialization::SerializeValue(&json, "flowDetectLine", FlowDetectLine);
			JsonSerialization::SerializeValue(&json, "queueDetectLine", QueueDetectLine);
			JsonSerialization::SerializeValue(&json, "laneLine1", LaneLine1);
			JsonSerialization::SerializeValue(&json, "laneLine2", LaneLine2);
			JsonSerialization::SerializeValue(&json, "flowRegion", FlowRegion);
			JsonSerialization::SerializeValue(&json, "queueRegion", QueueRegion);
			return json;
		}
	};

	//�¼���������
	enum class EventLaneType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Lane = 3,
		Bike = 4
	};

	//�¼�����
	enum class EventType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Congestion = 3,
		Retrograde = 4,
		Bike = 5
	};

	//�¼�����
	class EventLane
	{
	public:
		EventLane()
			:ChannelIndex(), LaneIndex(), LaneName(), LaneType(0), Line(), Region()
		{

		}
		//ͨ�����
		int ChannelIndex;
		//�������
		int LaneIndex;
		//��������
		std::string LaneName;
		//��������
		int LaneType;
		//��ͷ
		std::string Line;
		//����
		std::string Region;

		std::string ToJson() const
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelIndex", ChannelIndex);
			JsonSerialization::SerializeValue(&json, "laneIndex", LaneIndex);
			JsonSerialization::SerializeValue(&json, "laneName", LaneName);
			JsonSerialization::SerializeValue(&json, "laneType", LaneType);
			JsonSerialization::SerializeValue(&json, "region", Region);
			JsonSerialization::SerializeValue(&json, "line", Line);
			return json;
		}
	};

	//ͨ������
	enum class ChannelType
	{
		None = 0,
		GB28181 = 1,
		RTSP = 2,
		File = 3
	};

	//ͨ��״̬
	enum class ChannelStatus
	{
		//����
		Normal = 1,
		//�޷�����ƵԴ
		InputError = 2,
		//��Ƶ�������
		OutputError = 3,
		//�������쳣
		DecoderError = 4,
		//�޷���ȡ��Ƶ����
		ReadError = 5,
		//�������
		DecodeError = 6,
		//׼��ѭ������
		ReadEOF_Restart = 7,
		//�ļ����Ž���
		ReadEOF_Stop = 8,
		//���ڳ�ʼ��
		Init = 9,
		//�����쳣(����)
		Disconnect = 10,
		//ͨ����ͬ��(����)
		NotFoundChannel = 11,
		//������ͬ��(����)
		NotFoundLane = 12,
		//����֡û�д���
		NotHandle = 13,
		//����avc1��ʽ����
		FilterError = 14
	};

	//��Ƶͨ��
	class TrafficChannel
	{
	public:
		TrafficChannel()
			:ChannelIndex(0), ChannelName(), ChannelUrl(), ChannelType(0), DeviceId()
			, Loop(true), OutputDetect(false), OutputImage(false), OutputReport(false), OutputRecogn(false), GlobalDetect(false)
			, LaneWidth(0.0), ReportProperties(3), FreeSpeed(0.0)
			, ChannelStatus(0)
		{

		}
		//ͨ�����
		int ChannelIndex;
		//ͨ������
		std::string ChannelName;
		//ͨ����ַ
		std::string ChannelUrl;
		//ͨ������
		int ChannelType;
		//�����豸���
		std::string DeviceId;

		//�Ƿ�ѭ��
		bool Loop;
		//�Ƿ�����������
		bool OutputDetect;
		//�Ƿ����ͼƬ
		bool OutputImage;
		//�Ƿ������ⱨ��
		bool OutputReport;
		//�Ƿ����ʶ������
		bool OutputRecogn;
		//�Ƿ�ȫ�ּ��
		bool GlobalDetect;

		//�������
		double LaneWidth;
		//�ϱ�����
		int ReportProperties;
		//�������ٶ�(km/h)
		double FreeSpeed;

		//������������
		std::vector<FlowLane> FlowLanes;
		//�¼���������
		std::vector<EventLane> EventLanes;

		//ͨ��״̬
		int ChannelStatus;

		//�����ϱ������������Եı�ʶ
		const static int AllPropertiesFlag;

		/**
		* ��ȡͨ��rtmp��ַ
		* @return ͨ��rtmp��ַ
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://", ip, ":1935/live/", ChannelIndex);
		}

		/**
		* ��ȡͨ��http-flv��ַ
		* @return ͨ��http-flv��ַ
		*/
		std::string FlvUrl(const std::string ip) const
		{
			return StringEx::Combine("http://", ip, ":1936/live?port=1935&app=live&stream=", ChannelIndex);
		}

		std::string ToJson() const
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelIndex", ChannelIndex);
			JsonSerialization::SerializeValue(&json, "channelName", ChannelName);
			JsonSerialization::SerializeValue(&json, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&json, "channelType", ChannelType);
			JsonSerialization::SerializeValue(&json, "deviceId", DeviceId);
			JsonSerialization::SerializeValue(&json, "loop", Loop);
			JsonSerialization::SerializeValue(&json, "outputDetect", OutputDetect);
			JsonSerialization::SerializeValue(&json, "outputImage", OutputImage);
			JsonSerialization::SerializeValue(&json, "outputReport", OutputReport);
			JsonSerialization::SerializeValue(&json, "outputRecogn", OutputRecogn);
			JsonSerialization::SerializeValue(&json, "globalDetect", GlobalDetect);
			JsonSerialization::SerializeValue(&json, "laneWidth", LaneWidth);
			JsonSerialization::SerializeValue(&json, "reportProperties", ReportProperties);
			JsonSerialization::SerializeValue(&json, "freeSpeed", FreeSpeed);

			std::string flowLanesJson;
			for (std::vector<FlowLane>::const_iterator lit = FlowLanes.begin(); lit != FlowLanes.end(); ++lit)
			{
				JsonSerialization::AddClassItem(&flowLanesJson, lit->ToJson());
			}
			JsonSerialization::SerializeArray(&json, "flowLanes", flowLanesJson);

			std::string eventLanesJson;
			for (std::vector<EventLane>::const_iterator lit = EventLanes.begin(); lit != EventLanes.end(); ++lit)
			{
				JsonSerialization::AddClassItem(&eventLanesJson, lit->ToJson());
			}
			JsonSerialization::SerializeArray(&json, "eventLanes", eventLanesJson);
			return json;
		}
	};

	//�������
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

	//�����豸
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

	//����ͨ��
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

	//��ͨ״̬
	enum class TrafficStatus
	{
		Good = 1,
		Warning = 2,
		Bad = 3,
		Dead = 4
	};

	//������⻺��
	class FlowDetectCache
	{
	public:
		FlowDetectCache()
			:LastTimeStamp(0), LastHitPoint(), TotalDistance(0.0), MeterPerPixel(0.0), TotalTime(0)
		{

		}
		//��������һ�γ��ֵ�ʱ���
		long long LastTimeStamp;
		//��������һ�μ���
		Point LastHitPoint;
		//�ƶ��ܾ���(px)
		double TotalDistance;
		//ÿ���ش��������
		double MeterPerPixel;
		//�ƶ���ʱ��(ms)
		long long TotalTime;
	};

	//������������
	class FlowData
	{
	public:
		FlowData()
			: Id(0), LaneIndex(0), ChannelUrl(), LaneId(), LaneName(), IOIp(), IOIndex(0), Direction(0), FlowRegion(), QueueRegion(), ReportProperties(0)
			, Minute(0), TimeStamp(0)
			, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
			, Speed(0.0), TotalDistance(0.0), MeterPerPixel(0.0), TotalTime(0)
			, HeadDistance(0.0), TotalSpan(0), LastInRegion(0)
			, HeadSpace(0.0)
			, TimeOccupancy(0.0), TotalInTime(0)
			, QueueLength(0), MaxQueueLength(0), CurrentQueueLength(0), StopPoint()
			, SpaceOccupancy(0.0), TotalQueueLength(0), LaneLength(0.0), CountQueueLength(0)
			, TrafficStatus(0), FreeSpeed(0.0)
			, IoStatus(false)
		{

		}
		int Id;

		//�������
		int LaneIndex;
		//ͨ����ַ
		std::string ChannelUrl;
		//�������
		std::string LaneId;
		//��������
		std::string LaneName;
		//ioת����ip
		std::string IOIp;
		//ioת�����˿�
		int IOIndex;
		//��������
		int Direction;
		//�����������
		Polygon FlowRegion;
		//�ŶӼ������
		Polygon QueueRegion;
		//�ϱ�������
		int ReportProperties;

		//�ڼ�����
		int Minute;
		//ʱ���
		long long TimeStamp;

		//��������
		int Persons;
		//���г�����
		int Bikes;
		//Ħ�г�����
		int Motorcycles;
		//�γ�����
		int Cars;
		//���ֳ�����
		int Tricycles;
		//����������
		int Buss;
		//���������
		int Vans;
		//��������
		int Trucks;

		//ƽ���ٶ�(km/h)
		double Speed;
		//������ʻ�ܾ���(����ֵ) 
		double TotalDistance;
		//ÿ�����ش��������
		double MeterPerPixel;
		//������ʻ��ʱ��(����)
		long long TotalTime;

		//��ͷʱ��(sec)
		double HeadDistance;
		//������������ʱ���ĺ�(����)
		long long TotalSpan;
		//��һ���г����������ʱ���������ʱ��Ҫ
		long long LastInRegion;

		//��ͷ���(m)
		double HeadSpace;

		//ʱ��ռ����(%)
		double TimeOccupancy;
		//����ռ����ʱ��(����)
		long long TotalInTime;

		//�Ŷӳ���(m)
		int QueueLength;
		//�Ŷӳ���(m)
		int MaxQueueLength;
		//��ǰ˲ʱ�Ŷӳ���(m)������ʱ��Ҫ
		int CurrentQueueLength;
		//ֹͣ�߼��ĵ㣬����ʱ��Ҫ
		Point StopPoint;

		//�ռ�ռ����(%)
		double SpaceOccupancy;
		//�Ŷ��ܳ���(m)
		int TotalQueueLength;
		//��������
		double LaneLength;
		//�Ŷ��ܳ��ȶ�Ӧ�ļ�������
		int CountQueueLength;

		//��ͨ״̬
		int TrafficStatus;
		//�������ٶ�(km/h)
		double FreeSpeed;

		//io״̬
		bool IoStatus;

		//�����ڼ�����
		std::map<std::string, FlowDetectCache> Items;

		/**
		* ��ȡjson����
		* @return json����
		*/
		std::string ToJson()
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", LaneId);
			JsonSerialization::SerializeValue(&json, "laneName", LaneName);
			JsonSerialization::SerializeValue(&json, "direction", Direction);

			JsonSerialization::SerializeValue(&json, "dateTime", DateTime::ParseTimeStamp(TimeStamp).ToString());
			JsonSerialization::SerializeValue(&json, "minute", Minute);

			JsonSerialization::SerializeValue(&json, "persons", Persons);
			JsonSerialization::SerializeValue(&json, "bikes", Bikes);
			JsonSerialization::SerializeValue(&json, "motorcycles", Motorcycles);
			JsonSerialization::SerializeValue(&json, "cars", Cars);
			JsonSerialization::SerializeValue(&json, "tricycles", Tricycles);
			JsonSerialization::SerializeValue(&json, "buss", Buss);
			JsonSerialization::SerializeValue(&json, "vans", Vans);
			JsonSerialization::SerializeValue(&json, "trucks", Trucks);

			JsonSerialization::SerializeValue(&json, "averageSpeed", static_cast<int>(Speed));
			JsonSerialization::SerializeValue(&json, "headDistance", StringEx::Rounding(HeadDistance, 2));
			JsonSerialization::SerializeValue(&json, "headSpace", StringEx::Rounding(HeadSpace, 2));
			JsonSerialization::SerializeValue(&json, "timeOccupancy", static_cast<int>(TimeOccupancy));
			JsonSerialization::SerializeValue(&json, "trafficStatus", TrafficStatus);

			JsonSerialization::SerializeValue(&json, "queueLength", QueueLength);
			JsonSerialization::SerializeValue(&json, "spaceOccupancy", SpaceOccupancy);
			return json;
		}
	};

	//�������ṹ������
	class VehicleData
	{
	public:
		VehicleData()
			:Id(0), ChannelUrl(), LaneId(), LaneName(), TimeStamp(0), Guid(), Image(), Direction(0), Minute(0), Second(0), CarType(0), CarColor(0), CarBrand(), PlateType(0), PlateNumber()
		{

		}
		int Id;
		std::string ChannelUrl;
		std::string LaneId;
		std::string LaneName;
		long long TimeStamp;
		std::string Guid;
		std::string Image;
		int Direction;
		int Minute;
		int Second;
		int CarType;
		int CarColor;
		std::string CarBrand;
		int PlateType;
		std::string PlateNumber;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson(const std::string& ip)
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", LaneId);
			JsonSerialization::SerializeValue(&json, "laneName", LaneName);
			JsonSerialization::SerializeValue(&json, "direction", Direction);
			JsonSerialization::SerializeValue(&json, "dateTime", DateTime::ParseTimeStamp(TimeStamp).ToString());
			JsonSerialization::SerializeValue(&json, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&json, "image", TrafficDirectory::GetRecognUrl(ip, Guid));

			JsonSerialization::SerializeValue(&json, "carType", CarType);
			JsonSerialization::SerializeValue(&json, "carColor", CarColor);
			JsonSerialization::SerializeValue(&json, "carBrand", CarBrand);
			JsonSerialization::SerializeValue(&json, "plateType", PlateType);
			JsonSerialization::SerializeValue(&json, "plateNumber", PlateNumber);
			return json;
		}
	};

	//�ǻ������ṹ������
	class BikeData
	{
	public:
		BikeData()
			:Id(0), ChannelUrl(), LaneId(), LaneName(), TimeStamp(0), Guid(), Image(), Direction(0), Minute(0), Second(0), BikeType(0)
		{

		}
		int Id;
		std::string ChannelUrl;
		std::string LaneId;
		std::string LaneName;
		long long TimeStamp;
		std::string Guid;
		std::string Image;
		int Direction;
		int Minute;
		int Second;
		int BikeType;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson(const std::string& ip)
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", LaneId);
			JsonSerialization::SerializeValue(&json, "laneName", LaneName);
			JsonSerialization::SerializeValue(&json, "direction", Direction);
			JsonSerialization::SerializeValue(&json, "dateTime", DateTime::ParseTimeStamp(TimeStamp).ToString());
			JsonSerialization::SerializeValue(&json, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&json, "image", TrafficDirectory::GetRecognUrl(ip, Guid));

			JsonSerialization::SerializeValue(&json, "bikeType", BikeType);
			return json;
		}
	};

	//���˽ṹ������
	class PedestrainData
	{
	public:
		PedestrainData()
			:Id(0), ChannelUrl(), LaneId(), LaneName(), TimeStamp(0), Guid(), Image(), Direction(0), Minute(0), Second(0), Sex(0), Age(0), UpperColor(0)
		{

		}
		int Id;
		std::string ChannelUrl;
		std::string LaneId;
		std::string LaneName;
		long long TimeStamp;
		std::string Guid;
		std::string Image;
		int Direction;
		int Minute;
		int Second;
		int Sex;
		int Age;
		int UpperColor;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson(const std::string& ip)
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", LaneId);
			JsonSerialization::SerializeValue(&json, "laneName", LaneName);
			JsonSerialization::SerializeValue(&json, "direction", Direction);
			JsonSerialization::SerializeValue(&json, "dateTime", DateTime::ParseTimeStamp(TimeStamp).ToString());
			JsonSerialization::SerializeValue(&json, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&json, "image", TrafficDirectory::GetRecognUrl(ip, Guid));

			JsonSerialization::SerializeValue(&json, "sex", Sex);
			JsonSerialization::SerializeValue(&json, "age", Age);
			JsonSerialization::SerializeValue(&json, "upperColor", UpperColor);
			return json;
		}
	};

	//�¼�����
	class EventData
	{
	public:
		EventData()
			:Id(0), ChannelIndex(), ChannelUrl(), LaneIndex(0), TimeStamp(0), Guid(), Type(0)
		{

		}
		int Id;
		int ChannelIndex;
		std::string ChannelUrl;
		int LaneIndex;
		long long TimeStamp;
		std::string Guid;
		int Type;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson(const std::string& ip)
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&json, "laneIndex", LaneIndex);
			JsonSerialization::SerializeValue(&json, "dateTime", DateTime::ParseTimeStamp(TimeStamp).ToString());
			JsonSerialization::SerializeValue(&json, "type", Type);
			JsonSerialization::SerializeValue(&json, "image1", TrafficDirectory::GetImageLink(ip,Guid, 1));
			JsonSerialization::SerializeValue(&json, "image2", TrafficDirectory::GetImageLink(ip, Guid, 2));
			JsonSerialization::SerializeValue(&json, "video", TrafficDirectory::GetVideoLink(ip, Guid));
			return json;
		}
	};

	//�¼�ͳ��
	class EventStatistics
	{
	public:
		EventStatistics()
			:Type(0),Count(0)
		{

		}
		
		int Type;
		int Count;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson()
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "type", Type);
			JsonSerialization::SerializeValue(&json, "count", Count);
			return json;
		}
	};

	//io����
	class IoData
	{
	public:
		IoData()
			:ChannelUrl(), LaneId(), TimeStamp(0), Status(0)
		{

		}

		//��Ƶ��ַ
		std::string ChannelUrl;
		//�������
		std::string LaneId;
		//ʱ���
		long long TimeStamp;
		//io״̬
		int Status;

		std::string ToJson() const
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", LaneId);
			JsonSerialization::SerializeValue(&json, "dateTime", DateTime::ParseTimeStamp(TimeStamp).ToString());
			JsonSerialization::SerializeValue(&json, "status", Status);
			return json;
		}
	};

	//���Ԫ��״̬
	enum class DetectStatus
	{
		//��������
		Out,
		//�״ν���
		New,
		//��������
		In
	};

	//���Ԫ������
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

	//�����
	class DetectItem
	{
	public:
		DetectItem()
			:Id(), Region(), Type(DetectType::None), Status(DetectStatus::Out), Distance(0.0)
		{

		}
		std::string Id;
		//����
		//�������
		Rectangle Region;
		//���Ԫ������
		DetectType Type;

		//���
		//���Ԫ��״̬
		DetectStatus Status;
		//��������ֹͣ�߾���(px)
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

	//ʶ����
	class RecognItem
	{
	public:
		RecognItem()
			:ChannelIndex(0), FrameIndex(0), FrameSpan(0), TaskId(0), Guid(), Type(DetectType::None), Region(), Width(0), Height(0)
		{

		}
		//ͨ�����
		int ChannelIndex;
		//֡���
		int FrameIndex;
		//֡���ʱ��(����)
		unsigned char FrameSpan;
		//�����
		unsigned char TaskId;
		//guid
		std::string Guid;
		//���������
		DetectType Type;
		//�������
		Rectangle Region;
		//���
		int Width;
		//�߶�
		int Height;
	};

	//���ݿ�
	class TrafficData
	{
	public:
		/**
		* ���캯��
		* @param dbName ���ݿ��ַ
		*/
		TrafficData(const std::string& dbName = "flow.db");

		/**
		* ��ȡ���һ��������Ϣ
		* @return ���һ��������Ϣ
		*/
		std::string LastError();

		/**
		* �������ݿ�
		*/
		void UpdateDb();

		/**
		* ��ȡ����ֵ
		* @param key ������
		* @return ����ֵ
		*/
		std::string GetParameter(const std::string& key);

		/**
		* ��ȡ����ֵ
		* @param key ������
		* @param value ����ֵ
		* @return ���ý��
		*/
		bool SetParameter(const std::string& key, const std::string& value);

		/**
		* ��ȡ�������
		* @return �������
		*/
		GbParameter GetGbPrameter();

		/**
		* ���ù������ֵ
		* @param parameter �������
		* @return ���ý��
		*/
		bool SetGbPrameter(const GbParameter& parameter);

		/**
		* ��ȡ�����豸����
		* @return �����豸����
		*/
		std::vector<GbDevice> GetGbDevices();

		/**
		* ��ӹ����豸
		* @param device �����豸
		* @return ��ӳɹ������������򷵻�-1
		*/
		int InsertGbDevice(const GbDevice& device);

		/**
		* ���¹����豸
		* @param device �����豸
		* @return ���½��
		*/
		bool UpdateGbDevice(const GbDevice& device);

		/**
		* ɾ�������豸
		* @param deviceId �����豸���
		* @return ɾ�����
		*/
		bool DeleteGbDevice(int deviceId);

		/**
		* ��ѯ����ͨ������
		* @param deviceId �����豸���
		* @return ��ѯ���
		*/
		std::vector<GbChannel> GetGbChannels(const std::string& deviceId);

		/**
		* ��ѯͨ���б�
		* @return ͨ���б�
		*/
		std::vector<TrafficChannel> GetChannels();

		/**
		* ��ѯ����ͨ��
		* @param channelIndex ͨ�����
		* @return ͨ��
		*/
		TrafficChannel GetChannel(int channelIndex);

		/**
		* ����ͨ��
		* @param channel ͨ��
		* @return ���ý��
		*/
		bool SetChannel(const TrafficChannel& channel);

		/**
		* ����ͨ������
		* @param channels ͨ������
		* @return ���ý��
		*/
		bool SetChannels(const std::vector<TrafficChannel>& channels);

		/**
		* ɾ��ͨ��
		* @param channel ͨ��
		* @return ɾ�����
		*/
		bool DeleteChannel(int channelIndex);

		/**
		* ���ͨ��
		*/
		void ClearChannels();

		/**
		* ��ѯ�����б�
		* @param channelIndex ͨ�����
		* @param laneId �������
		* @return �����б�
		*/
		std::vector<FlowLane> GetFlowLanes(int channelIndex, const std::string& laneId);

		/**
		* �����������
		* @param data ͨ�����
		* @return �������
		*/
		bool InsertFlowData(const FlowData& data);

		/**
		* ��ѯ��������
		* @param channelUrl ͨ����ַ��Ϊ��ʱ��ѯ����
		* @param laneId ������ţ�Ϊ��ʱ��ѯ����
		* @param startTime ��ʼʱ�䣬Ϊ0ʱ��ѯ����
		* @param endTime ����ʱ�䣬Ϊ0ʱ��ѯ����
		* @param pageNum ��ҳҳ�룬Ϊ0ʱ��ѯ����
		* @param pageSize ÿҳ������Ϊ0ʱ��ѯ����
		* @return �������
		*/
		std::tuple<std::vector<FlowData>, int> GetFlowDatas(const std::string& channelUrl, const std::string& laneId, long long startTime, long long endTime, int pageNum, int pageSize);
		
		/**
		* ɾ����������
		* @param keepDay ��������
		* @return �������
		*/
		void DeleteFlowDatas(int keepDay);

		/**
		* ��ӻ������ṹ������
		* @param data ͨ�����
		* @return �������
		*/
		bool InsertVehicleData(const VehicleData& data);

		/**
		* ��ѯ�������ṹ������
		* @param channelUrl ͨ����ַ��Ϊ��ʱ��ѯ����
		* @param laneId ������ţ�Ϊ��ʱ��ѯ����
		* @param carType �������ͣ�Ϊ0ʱ��ѯ����
		* @param startTime ��ʼʱ�䣬Ϊ0ʱ��ѯ����
		* @param endTime ����ʱ�䣬Ϊ0ʱ��ѯ����
		* @param pageNum ��ҳҳ�룬Ϊ0ʱ��ѯ����
		* @param pageSize ÿҳ������Ϊ0ʱ��ѯ����
		* @return �������
		*/
		std::tuple<std::vector<VehicleData>, int> GetVehicleDatas(const std::string& channelUrl, const std::string& laneId,int carType, long long startTime, long long endTime, int pageNum, int pageSize);
		
		/**
		* ɾ���������ṹ������
		* @param keepDay ��������
		* @return �������
		*/
		void DeleteVehicleDatas(int keepCount);

		/**
		* ��ӷǻ������ṹ������
		* @param data ͨ�����
		* @return �������
		*/
		bool InsertBikeData(const BikeData& data);

		/**
		* ��ѯ�ǻ������ṹ������
		* @param channelUrl ͨ����ַ��Ϊ��ʱ��ѯ����
		* @param laneId ������ţ�Ϊ��ʱ��ѯ����
		* @param bikeType �ǻ��������ͣ�Ϊ0ʱ��ѯ����
		* @param startTime ��ʼʱ�䣬Ϊ0ʱ��ѯ����
		* @param endTime ����ʱ�䣬Ϊ0ʱ��ѯ����
		* @param pageNum ��ҳҳ�룬Ϊ0ʱ��ѯ����
		* @param pageSize ÿҳ������Ϊ0ʱ��ѯ����
		* @return �������
		*/
		std::tuple<std::vector<BikeData>, int> GetBikeDatas(const std::string& channelUrl, const std::string& laneId,int bikeType, long long startTime, long long endTime, int pageNum, int pageSize);
		
		/**
		* ɾ���ǻ������ṹ������
		* @param keepDay ��������
		* @return �������
		*/
		void DeleteBikeDatas(int keepCount);

		/**
		* ������˽ṹ������
		* @param data ͨ�����
		* @return �������
		*/
		bool InsertPedestrainData(const PedestrainData& data);

		/**
		* ��ѯ���˽ṹ������
		* @param channelUrl ͨ����ַ��Ϊ��ʱ��ѯ����
		* @param laneId ������ţ�Ϊ��ʱ��ѯ����
		* @param startTime ��ʼʱ�䣬Ϊ0ʱ��ѯ����
		* @param endTime ����ʱ�䣬Ϊ0ʱ��ѯ����
		* @param pageNum ��ҳҳ�룬Ϊ0ʱ��ѯ����
		* @param pageSize ÿҳ������Ϊ0ʱ��ѯ����
		* @return �������
		*/
		std::tuple<std::vector<PedestrainData>, int> GetPedestrainDatas(const std::string& channelUrl, const std::string& laneId, long long startTime, long long endTime, int pageNum, int pageSize);
		
		/**
		* ɾ�����˽ṹ������
		* @param keepDay ��������
		* @return �������
		*/
		void DeletePedestrainDatas(int keepCount);

		/**
		* ����¼�����
		* @param data ͨ�����
		* @return �������
		*/
		bool InsertEventData(const EventData& data);

		/**
		* ���캯��
		* @param channelUrl ��Ƶ��ַ��Ϊ��ʱ��ѯ����
		* @param type �¼�����
		* @param startTime ��ʼʱ�����Ϊ0ʱ��ѯ����
		* @param endTime ����ʱ���,Ϊ0ʱ��ѯ����
		* @param pageNum ��ҳҳ��,0��ʾ��ѯ����
		* @param pageSize ��ҳ����,0��ʾ��ѯ����
		* @return �¼����ݼ���
		*/
		std::tuple<std::vector<EventData>, int> GetEventDatas(const std::string& channelUrl,int type, long long startTime, long long endTime, int pageNum, int pageSize);
		
		/**
		* ���캯��
		* @param channelUrl ��Ƶ��ַ��Ϊ��ʱ��ѯ����
		* @param type �¼�����
		* @param startTime ��ʼʱ�����Ϊ0ʱ��ѯ����
		* @param endTime ����ʱ���,Ϊ0ʱ��ѯ����
		* @return �¼����ݼ���
		*/
		std::vector<EventStatistics> GetEventStatistics(const std::string& channelUrl, int type, long long startTime, long long endTime);

		/**
		* ɾ���¼�����
		* @param keepDay ��������
		* @return �������
		*/
		void DeleteEventData(int keepCount);

	private:
		/**
		* ���ͨ��
		* @param sqlite ��ѯ���
		* @param channel Ҫ����ͨ������
		*/
		TrafficChannel FillChannel(const SqliteReader& sqlite);

		/**
		* �����������
		* @param sqlite ��ѯ���
		* @return ��������
		*/
		FlowLane FillFlowLane(const SqliteReader& sqlite);

		/**
		* ����¼�����
		* @param sqlite ��ѯ���
		* @return �¼�����
		*/
		EventLane FillEventLane(const SqliteReader& sqlite);

		/**
		* ���ͨ��
		* @param channel ͨ��
		* @return ��ӽ��
		*/
		bool InsertChannel(const TrafficChannel& channel);

		//���ݿ��ַ
		std::string _dbName;

		//����д��
		SqliteWriter _sqlite;
	};


}



