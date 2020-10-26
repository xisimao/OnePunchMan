#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//��ͨ״̬
	enum class TrafficStatus
	{
		Good = 1,
		Normal = 2,
		Warning = 3,
		Bad = 4,
		Dead = 5
	};

	//��������
	class FlowLane:public TrafficLane
	{
	public:
		FlowLane()
			:TrafficLane(),LaneId(),Direction(0),Region(),DetectLine(),StopLine(),LaneType(0)
			,FlowDirection(0),Length(0),IOIp(),IOPort(0),IOIndex(0), ReportProperties(0)
		{

		}
		//�������
		std::string LaneId;
		//��������
		int Direction;
		//����
		std::string Region;
		//�����
		std::string DetectLine;
		//ֹͣ��
		std::string StopLine;
		//��������
		int LaneType;
		//��������
		int FlowDirection;
		//��������
		int Length;
		//io�������ַ
		std::string IOIp;
		//io������˿�
		int IOPort;
		//io����������
		int IOIndex;	
		//�ϱ�����
		int ReportProperties;
	};

	//������Ƶͨ��
	class FlowChannel:public TrafficChannel
	{
	public:
		//��������
		std::vector<FlowLane> Lanes;
	};

	//������Ƶͨ�����ݿ����
	class FlowChannelData:public TrafficData
	{
	public:
		/**
		* ��ѯͨ���б�
		* @return ͨ���б�
		*/
		std::vector<FlowChannel> GetList();

		/**
		* ��ѯ�����б�
		* @param channelIndex ͨ�����
		* @param laneId �������
		* @return �����б�
		*/
		std::vector<FlowLane> GetLaneList(int channelIndex,const std::string& laneId);

		/**
		* ��ѯ����ͨ��
		* @param channelIndex ͨ�����
		* @return ͨ��
		*/
		FlowChannel Get(int channelIndex);

		/**
		* ����ͨ��
		* @param channel ͨ��
		* @return ���ý��
		*/
		bool Set(const FlowChannel& channel);

		/**
		* ����ͨ������
		* @param channels ͨ������
		* @return ���ý��
		*/
		bool SetList(const std::vector<FlowChannel>& channels);
		
		/**
		* ɾ��ͨ��
		* @param channel ͨ��
		* @return ɾ�����
		*/
		bool Delete(int channelIndex);

		/**
		* ���ͨ��
		*/
		void Clear();

		void UpdateDb();

	private:
		/**
		* ���ͨ��
		* @param channel ͨ��
		* @return ��ӽ��
		*/
		bool InsertChannel(const FlowChannel& channel);

		/**
		* ��ӳ���
		* @param lane ����
		* @return ��ӽ��
		*/
		bool InsertLane(const FlowLane& lane);

		/**
		* ���ͨ��
		* @param sqlite ��ѯ���
		* @return ͨ��
		*/
		FlowChannel FillChannel(const SqliteReader& sqlite);

		/**
		* ��䳵��
		* @param sqlite ��ѯ���
		* @return ����
		*/
		FlowLane FillLane(const SqliteReader& sqlite);
	};

	//������������
	class FlowReportData
	{
	public:
		FlowReportData()
			: ChannelUrl(),LaneId(), LaneName(), Direction(0), ReportProperties(0)
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
		* ��ȡ������ʾ���ַ���
		* @return ������ʾ���ַ���
		*/
		std::string ToString()
		{
			return StringEx::Combine("channel url:", ChannelUrl, " lane:", LaneId, " timestamp:", DateTime::ParseTimeStamp(TimeStamp).ToString(), " properties:", ReportProperties, " cars:", Cars, " tricycles:", Tricycles, " buss", Buss, " vans:", Vans, " trucks:", Trucks, " bikes:", Bikes, " motorcycles:", Motorcycles, " persons:", Persons, " speed(km/h):", Speed, " head distance(sec):", HeadDistance, " head space(m)", HeadSpace, " time occ(%):", TimeOccupancy, " traffic status:", TrafficStatus, " queue length(m):", QueueLength, " pace occ(%):", SpaceOccupancy);
		}

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
			JsonSerialization::SerializeValue(&messageJson, "headDistance", HeadDistance);
			JsonSerialization::SerializeValue(&messageJson, "headSpace", HeadSpace);
			JsonSerialization::SerializeValue(&messageJson, "timeOccupancy", static_cast<int>(TimeOccupancy));
			JsonSerialization::SerializeValue(&messageJson, "trafficStatus", TrafficStatus);

			JsonSerialization::SerializeValue(&messageJson, "queueLength", QueueLength);
			JsonSerialization::SerializeValue(&messageJson, "spaceOccupancy", SpaceOccupancy);
			return messageJson;
		}
	};
}


