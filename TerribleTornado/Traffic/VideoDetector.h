#pragma once
#include <queue>
#include <vector>

#include "FlowChannelData.h"
#include "LaneDetector.h"
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

	//��Ƶ���ݼ���
	class VideoDetector
	{
	public:

		/**
		* @brief: ���캯��
		*/
		VideoDetector();

		/**
		* @brief: ��������
		*/
		~VideoDetector();

		/**
		* @brief: ���³�������
		* @param: lanes ��������
		*/
		void UpdateLanes(const std::vector<Lane>& lanes);

		/**
		* @brief: �������
		* @param: items ������ݼ���
		* @param: timeStamp ʱ���
		* @return: �ı��IO״̬����
		*/
		std::vector<IOItem> Detect(const std::map<std::string,DetectItem>& items,long long timeStamp);

		/**
		* @brief: ��������Ƿ��ڳ�����Χ��
		* @param: item �����
		* @return: �ڳ����ڷ��س�����ţ����򷵻ؿ��ַ���
		*/
		std::string Contains(const DetectItem& item);

		/**
		* @brief: �ռ���Ƶͨ����������
		* @return: ��Ƶͨ����������
		*/
		std::vector<LaneItem> Collect();

		//ͨ����ַ
		std::string Url;

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
		std::vector<LaneDetector*> _lanes;
	};
}


