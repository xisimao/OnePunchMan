#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//�������
	enum class VideoStructType
	{
		Vehicle = 1,
		Bike = 2,
		Pedestrain = 3
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

	//��������
	class FlowLane:public TrafficLane
	{
	public:
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
	};

	//������Ƶͨ��
	class FlowChannel:public TrafficChannel
	{
	public:
		//��������
		std::vector<FlowLane> Lanes;
	};

	//������Ƶͨ�����ݿ����
	class FlowChannelData
	{
	public:

		FlowChannelData();

		/**
		* @brief: ��ѯͨ���б�
		* @return: ͨ���б�
		*/
		std::vector<FlowChannel> GetList();

		/**
		* @brief: ��ѯ����ͨ��
		* @param: channelIndex ͨ�����
		* @return: ͨ��
		*/
		FlowChannel Get(int channelIndex);

		/**
		* @brief: ���ͨ��
		* @param: channel ͨ��
		* @return: ��ӽ��
		*/
		bool Insert(const FlowChannel& channel);

		/**
		* @brief: ����ͨ��
		* @param: channel ͨ��
		* @return: ���ý��
		*/
		bool Set(const FlowChannel& channel);

		/**
		* @brief: ����ͨ������
		* @param: channels ͨ������
		* @return: ���ý��
		*/
		bool SetList(const std::vector<FlowChannel>& channels);
		
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
		FlowChannel FillChannel(const SqliteReader& sqlite);

		//����д��
		SqliteWriter _sqlite;
	};
}


