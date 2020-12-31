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

		static std::string GetDataFrameJpg(int channelIndex,int frameIndex)
		{
			return StringEx::Combine(TrafficDirectory::DataDir, "images/", channelIndex, "_", frameIndex, ".jpg");
		}

		static std::string GetTempFrameJpg(int channelIndex, int frameIndex)
		{
			return 	StringEx::Combine(TrafficDirectory::TempDir,channelIndex, "_", frameIndex, ".jpg");
		}

		static std::string GetAllTempFrameJpg(int channelIndex)
		{
			return 	StringEx::Combine(TrafficDirectory::TempDir, channelIndex, "_*.jpg");
		}

		static std::string GetChannelJpg(int channelIndex)
		{
			return StringEx::Combine(TrafficDirectory::FileDir,"channel_", channelIndex, ".jpg");
		}

		static std::string GetRecognBmp(const std::string& id)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, "recogn_", id, ".bmp");
		}

		static std::string GetEventJpg(const std::string& id,int imageIndex)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, "event_",id, "_", imageIndex, ".jpg");
		}

		static std::string GetEventMp4(const std::string& id)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, "event_", id, ".mp4");
		}

		static std::string GetImageLink(const std::string& id, int imageIndex)
		{
			return StringEx::Combine(TrafficDirectory::FileLink, "/event_", id, "_", imageIndex, ".jpg");
		}

		static std::string GetVideoLink(const std::string& id)
		{
			return StringEx::Combine(TrafficDirectory::FileLink, "/event_", id, ".mp4");
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
			, LaneWidth(0.0), ReportProperties(3),FreeSpeed(0.0)
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
			:LastTimeStamp(0), LastHitPoint(), TotalDistance(0.0), MeterPerPixel(0.0),TotalTime(0)
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
			: Id(0),LaneIndex(0),ChannelUrl(), LaneId(), LaneName(), IOIp(),IOIndex(0), Direction(0), FlowRegion(), QueueRegion(),ReportProperties(0)
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
		* ��ȡexcel�õ�json
		* @return excel�õ�json
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
		* ��ȡmqtt�����õ�json
		* @return mqtt�����õ�json
		*/
		std::string ToMessageJson()
		{
			std::string messageJson;
			JsonSerialization::SerializeValue(&messageJson, "channelUrl", ChannelUrl);
			JsonSerialization::SerializeValue(&messageJson, "laneId", LaneId);
			DateTime time = DateTime::ParseTimeStamp(TimeStamp);
			DateTime adjustTime = DateTime(time.Year() + 1, time.Month(), time.Day(), time.Hour(), time.Minute(), time.Second(), time.Millisecond());
			JsonSerialization::SerializeValue(&messageJson, "timeStamp", adjustTime.TimeStamp());

			JsonSerialization::SerializeValue(&messageJson, "persons", Persons);
			JsonSerialization::SerializeValue(&messageJson, "bikes", Bikes);
			JsonSerialization::SerializeValue(&messageJson, "motorcycles", Motorcycles);
			JsonSerialization::SerializeValue(&messageJson, "cars", Cars);
			JsonSerialization::SerializeValue(&messageJson, "tricycles", Tricycles);
			JsonSerialization::SerializeValue(&messageJson, "buss", Buss);
			JsonSerialization::SerializeValue(&messageJson, "vans", Vans);
			JsonSerialization::SerializeValue(&messageJson, "trucks", Trucks);

			JsonSerialization::SerializeValue(&messageJson, "averageSpeed", static_cast<int>(Speed));
			JsonSerialization::SerializeValue(&messageJson, "headDistance", StringEx::Rounding(HeadDistance, 2));
			JsonSerialization::SerializeValue(&messageJson, "headSpace", StringEx::Rounding(HeadSpace, 2));
			JsonSerialization::SerializeValue(&messageJson, "timeOccupancy", static_cast<int>(TimeOccupancy));
			JsonSerialization::SerializeValue(&messageJson, "trafficStatus", TrafficStatus);

			JsonSerialization::SerializeValue(&messageJson, "queueLength", QueueLength);
			JsonSerialization::SerializeValue(&messageJson, "spaceOccupancy", SpaceOccupancy);
			return messageJson;
		}
	};

	//�������
	enum class VideoStructType
	{
		Vehicle = 1,
		Bike = 2,
		Pedestrain = 3
	};

	//�������ṹ������
	class VehicleData
	{
	public:
		VehicleData()
			:Id(0),ChannelUrl(),LaneId(), LaneName(), TimeStamp(0), Guid(),Image(), Direction(0), Minute(0), Second(0), CarType(0), CarColor(0), CarBrand(), PlateType(0), PlateNumber()
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

	//���˽ṹ������
	class PedestrainData
	{
	public:
		PedestrainData()
			:Id(0), ChannelUrl(), LaneId(), LaneName(), TimeStamp(0),Guid(), Image(), Direction(0), Minute(0), Second(0), Sex(0), Age(0), UpperColor(0)
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

	//�¼�����
	class EventData
	{
	public:
		EventData()
			:Id(0), ChannelIndex(), ChannelUrl(), LaneIndex(0), TimeStamp(0), Type(0)
		{

		}
		int Id;
		std::string Guid;
		int ChannelIndex;
		std::string ChannelUrl;
		int LaneIndex;
		long long TimeStamp;
		int Type;
	};

	//���ݿ�
	class TrafficData
	{
	public:
		/**
		* ���캯��
		* @param dbName ���ݿ��ַ
		*/
		TrafficData(const std::string& dbName="flow.db");

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

		bool InsertFlowData(const FlowData& data);

		std::vector<FlowData> GetFlowDatas(const std::string& channelUrl, const std::string& laneId, int pageNum, int pageSize);

		void DeleteFlowDatas(int keeyDay);

		bool InsertVehicleData(const VehicleData& data);

		std::vector<VehicleData> GetVehicleDatas(const std::string& channelUrl, const std::string& laneId, int pageNum, int pageSize);

		void DeleteVehicleDatas(int keepCount);
		
		bool InsertBikeData(const BikeData& data);

		std::vector<BikeData> GetBikeDatas(const std::string& channelUrl, const std::string& laneId, int pageNum, int pageSize);

		void DeleteBikeDatas(int keepCount);

		bool InsertPedestrainData(const PedestrainData& data);

		std::vector<PedestrainData> GetPedestrainDatas(const std::string& channelUrl, const std::string& laneId, int pageNum, int pageSize);
		
		void DeletePedestrainDatas(int keepCount);

		/**
		* ���캯��
		* @param channelIndex ��Ƶ���,0��ʾ��ѯ����
		* @param pageNum ��ҳҳ��,0��ʾ��ѯ����
		* @param pageSize ��ҳ����,0��ʾ��ѯ����
		* @return �¼����ݼ���
		*/
		std::vector<EventData> GetEventDatas(int channelIndex,int pageNum,int pageSize);

		bool InsertEventData(const EventData& data);

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



