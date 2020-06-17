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
		Lane = 3
	};

	//�¼�����
	class EventLane:public TrafficLane
	{
	public:
		//��������
		int LaneType;
		//��ͷ
		std::string Line;
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
		* @brief: ��ѯͨ���б�
		* @return: ͨ���б�
		*/
		std::vector<EventChannel> GetList();

		/**
		* @brief: ��ѯ����ͨ��
		* @param: channelIndex ͨ�����
		* @return: ͨ��
		*/
		EventChannel Get(int channelIndex);

		/**
		* @brief: ���ͨ��
		* @param: channel ͨ��
		* @return: ��ӽ��
		*/
		bool Insert(const EventChannel& channel);

		/**
		* @brief: ����ͨ��
		* @param: channel ͨ��
		* @return: ���ý��
		*/
		bool Set(const EventChannel& channel);

		/**
		* @brief: ����ͨ������
		* @param: channels ͨ������
		* @return: ���ý��
		*/
		bool SetList(const std::vector<EventChannel>& channels);
		
		/**
		* @brief: ɾ��ͨ��
		* @param: channel ͨ��
		* @return: ɾ�����
		*/
		bool Delete(int channelIndex);

		/**
		* @brief: ���ͨ��
		*/
		void Clear();

		void UpdateDb();

	private:
		/**
		* @brief: ���ͨ��
		* @param: sqlite ��ѯ���
		* @return: ͨ��
		*/
		EventChannel FillChannel(const SqliteReader& sqlite);
	};
}


