#pragma once
#include <string>
#include <math.h>

#include "StringEx.h"
#include "Sqlite.h"

namespace OnePunchMan
{
	//��������
	class FlowLane 
	{
	public:
		FlowLane()
			: ChannelIndex(), LaneIndex(), LaneName(), LaneId(), Direction(0), FlowDirection(0)
			, StopLine(), FlowDetectLine(), QueueDetectLine(), LaneLine1(), LaneLine2(), FlowRegion(),QueueRegion()
			, IOIp(), IOPort(0), IOIndex(0), ReportProperties(0)
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
			, LaneWidth(0.0), ReportProperties(3)
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

		//������������
		std::vector<FlowLane> FlowLanes;
		//�¼���������
		std::vector<EventLane> EventLanes;

		//ͨ��״̬
		int ChannelStatus;

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

	//��ͨ״̬
	enum class TrafficStatus
	{
		Good = 1,
		Normal = 2,
		Warning = 3,
		Bad = 4,
		Dead = 5
	};

	//������������
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
		//ͨ����ַ
		std::string ChannelUrl;
		//�������
		std::string LaneId;
		//��������
		std::string LaneName;
		//��������
		int Direction;
		//��Ҫ�ϱ�������
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
		//ʱ��ռ����(%)
		double TimeOccupancy;
		//����ʱ��(sec)
		double HeadDistance;
		//��ͷ���(m)
		double HeadSpace;
		//��ͨ״̬
		int TrafficStatus;

		//�Ŷӳ���(m)
		int QueueLength;
		//�ռ�ռ����(%)
		double SpaceOccupancy;

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

	//������Ϣ
	class TrafficDirectory
	{
	public:
		//�ļ���ʱ���Ŀ¼
		static std::string TempDir;
		//���ݴ��Ŀ¼
		static std::string DataDir;
		//�ļ����Ŀ¼
		static std::string FileDir;
		//�ļ�Ŀ¼����
		static std::string FileLink;
		//ҳ��Ŀ¼
		static std::string WebDir;

		/**
		* ��ʼ��Ŀ¼����
		* @param jd json����
		*/
		static void Init(const std::string& webDir)
		{
			WebDir = webDir;
		}
	};

	//���ݿ�
	class TrafficData
	{
	public:
		/**
		* ���캯��
		* @param dbName ���ݿ��ַ
		*/
		TrafficData(const std::string& dbName="traffic.db");

		/**
		* ��ȡ���һ��������Ϣ
		* @return ���һ��������Ϣ
		*/
		std::string LastError();

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
		* ���ù������ֵ
		* @param parameter �������
		* @return ���ý��
		*/
		bool SetGbPrameter(const GbParameter& parameter);

		/**
		* ��ȡ�������
		* @return �������
		*/
		GbParameter GetGbPrameter();

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
		* �������ݿ�
		*/
		virtual void UpdateDb();

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



