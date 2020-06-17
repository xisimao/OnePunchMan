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
		//�������
		std::string LaneId;
		//��������
		int Direction;
		//�����
		std::string DetectLine;
		//ֹͣ��
		std::string StopLine;
		//������1
		std::string LaneLine1;
		//������2
		std::string LaneLine2;
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
	class FlowChannelData:public TrafficData
	{
	public:
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
		FlowChannel FillChannel(const SqliteReader& sqlite);
	};
}


