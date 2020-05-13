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
	};

	//�¼���Ƶͨ��
	class EventChannel:public TrafficChannel
	{
	public:
		//��������
		std::vector<EventLane> Lanes;

	};

	//������Ƶͨ�����ݿ����
	class EventChannelData
	{
	public:

		EventChannelData();

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
		* @brief: ����ͨ��
		* @param: channel ͨ��
		* @return: ���ӽ��
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
		* @return: ɾ���ṹ
		*/
		bool Delete(int channelIndex);

		/**
		* @brief: ���ͨ��
		*/
		void Clear();

		/**
		* @brief: ��ȡ���һ��������Ϣ
		* @return: ���һ��������Ϣ
		*/
		std::string LastError();

	private:

		//���ݿ�����
		static const std::string DbName;

		/**
		* @brief: ���ͨ��
		* @param: sqlite ��ѯ���
		* @return: ͨ��
		*/
		EventChannel FillChannel(const SqliteReader& sqlite);

		//����д��
		SqliteWriter _sqlite;
	};
}

