#pragma once
#include <map>
#include <string>
#include <mutex>

#include "Shape.h"
#include "LogPool.h"
#include "MqttChannel.h"

namespace OnePunchMan
{
	//�������滺��
	class FlowReportData
	{
	public:
		FlowReportData()
			: ChannelUrl(),LaneId(), TimeStamp(0), ReportProperties(0)
			, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
			, Speed(0.0), TimeOccupancy(0.0), HeadDistance(0.0), HeadSpace(0.0), TrafficStatus(0)
			, QueueLength(0), SpaceOccupancy(0.0)
		{

		}
		//ͨ����ַ
		std::string ChannelUrl;
		//�������
		std::string LaneId;
		//ʱ���
		long long TimeStamp;
		//��Ҫ�ϱ�������
		int ReportProperties;

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

	};
	
	//���ݺϲ��ֵ�
	class DataMergeMap
	{
	public:
		/**
		* ���캯��
		* @param mqtt mqtt
		*/
		DataMergeMap(MqttChannel* mqtt);

		/**
		* ������������
		* @param data ��������
		*/
		void PushData(const FlowReportData& data);

	private:
		//����mqtt����
		static const std::string FlowTopic;

		//mqtt
		MqttChannel* _mqtt;

		//ͬ����
		std::mutex _mutex;
		//���������ֵ�
		std::map<std::string, FlowReportData> _datas;
	};

}


