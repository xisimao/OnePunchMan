#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//�¼���������
	enum class EventLaneType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Lane = 3,
		Bike=4
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
	class EventLane:public TrafficLane
	{
	public:
		EventLane()
			:TrafficLane(),LaneType(0),Line(),Region()
		{

		}
		//��������
		int LaneType;
		//��ͷ
		std::string Line;
		//����
		std::string Region;
	};
	
	//�¼���Ƶͨ��
	class EventChannel:public TrafficChannel
	{
	public:
		//��������
		std::vector<EventLane> Lanes;

	};

	//������Ƶͨ�����ݿ����
	class EventChannelData:public TrafficData
	{
	public:
		/**
		* ��ѯͨ���б�
		* @return ͨ���б�
		*/
		std::vector<EventChannel> GetList();

		/**
		* ��ѯ����ͨ��
		* @param channelIndex ͨ�����
		* @return ͨ��
		*/
		EventChannel Get(int channelIndex);



		/**
		* ����ͨ��
		* @param channel ͨ��
		* @return ���ý��
		*/
		bool Set(const EventChannel& channel);

		/**
		* ����ͨ������
		* @param channels ͨ������
		* @return ���ý��
		*/
		bool SetList(const std::vector<EventChannel>& channels);
		
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
		* @param sqlite ��ѯ���
		* @return ͨ��
		*/
		EventChannel FillChannel(const SqliteReader& sqlite);

		/**
		* ���ͨ��
		* @param channel ͨ��
		* @return ��ӽ��
		*/
		bool InsertChannel(const EventChannel& channel);
	};
}


