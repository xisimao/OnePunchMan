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
}


