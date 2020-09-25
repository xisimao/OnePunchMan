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
			,FlowDirection(0),Length(0),IOIp(),IOPort(0),IOIndex(0)
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
		* @param: channel ͨ��
		* @return: ��ӽ��
		*/
		bool InsertChannel(const FlowChannel& channel);

		/**
		* @brief: ��ӳ���
		* @param: lane ����
		* @return: ��ӽ��
		*/
		bool InsertLane(const FlowLane& lane);

		/**
		* @brief: ���ͨ��
		* @param: sqlite ��ѯ���
		* @return: ͨ��
		*/
		FlowChannel FillChannel(const SqliteReader& sqlite);

		/**
		* @brief: ��䳵��
		* @param: sqlite ��ѯ���
		* @return: ����
		*/
		FlowLane FillLane(const SqliteReader& sqlite);
	};
}


