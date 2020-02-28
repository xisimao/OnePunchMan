#pragma once
#include <queue>
#include <vector>

#include "Lane.h"
#include "Thread.h"

namespace Saitama
{
	//IO״̬
	class IOItem
	{
	public:

		/**
		* @brief: ���캯��
		*/
		IOItem()
			:IOItem(std::string(),0, std::string(),0,0)
		{

		}

		/**
		* @brief: ���캯��
		* @param: channelId ͨ��Id
		* @param: channelIndex ͨ�����
		* @param: laneId �������
		* @param: laneIndex �������
		* @param: status ����IO״̬
		*/
		IOItem(const std::string& channelId,int channelIndex,const std::string& laneId,int laneIndex, int status)
			:ChannelId(channelId),ChannelIndex(channelIndex), LaneId(laneId),LaneIndex(laneIndex),Status(status)
		{

		}
		//ͨ�����
		std::string ChannelId;
		//ͨ�����
		int ChannelIndex;
		//�������
		std::string LaneId;
		//�������
		int LaneIndex;
		//IO״̬
		int Status;
	};

	//��Ƶͨ�������߳�
	class Channel
	{
	public:

		/**
		* @brief: ���캯��
		*/
		Channel();

		/**
		* @brief: ��������
		*/
		~Channel();

		/**
		* @brief: ���³�������
		* @param: lanes ��������
		*/
		void UpdateLanes(const std::vector<Lane*>& lanes);

		/**
		* @brief: �������
		* @param: vehicles ������������ݼ���
		* @param: bikes �ǻ�����������ݼ���
		* @param: pedestrains ���˼�����ݼ���
		* @return: �ı��IO״̬����
		*/
		std::vector<IOItem> Detect(const std::vector<DetectItem>& vehicles,const std::vector<DetectItem>& bikes,std::vector<DetectItem>& pedestrains);

		/**
		* @brief: �ռ���Ƶͨ����������
		* @return: ��Ƶͨ����������
		*/
		std::vector<LaneItem> Collect();

		//ͨ�����
		std::string Id;

		//ͨ�����
		int Index;

	private:

		/**
		* @brief: ��ճ�������
		* @return: �������
		*/
		void ClearLanes();

		//ͬ����
		std::mutex _dataMutex;
		//���ݶ���
		std::queue<std::string> _datas;

		//ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<Lane*> _lanes;
	};
}


