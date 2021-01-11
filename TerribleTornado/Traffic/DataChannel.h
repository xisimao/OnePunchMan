#pragma once
#include "Thread.h"
#include "Sqlite.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//�¼����ݲ����߳�
	class DataChannel:public ThreadObject
	{
	public:
		/**
		* ���캯��
		*/
		DataChannel();

		/**
		* ��ʼ�����ò���
		* @param jd ���ò���
		*/
		static void Init(const JsonDeserialization& jd);

		/**
		* ����µ��¼�����
		* @param data �¼�����
		*/
		void PushEventData(const EventData& data);

		/**
		* ����µ��¼�����
		* @param data �¼�����
		*/
		void PushFlowData(const FlowData& data);

		/**
		* ����µ��¼�����
		* @param data �¼�����
		*/
		void PushVehicleData(const VehicleData& data);

		/**
		* ����µ��¼�����
		* @param data �¼�����
		*/
		void PushBikeData(const BikeData& data);

		/**
		* ����µ��¼�����
		* @param data �¼�����
		*/
		void PushPedestrainData(const PedestrainData& data);

	protected:
		void StartCore();

	private:
		//��ౣ����¼���������
		static int MaxDataCount;

		//��������
		std::mutex _flowMutex;
		//���ֵ����������ֵ�
		std::map<std::string, FlowData> _mergeFlowDatas;
		//�������������ݶ���
		std::vector<FlowData> _flowDatas;

		//����������
		std::mutex _vehicleMutex;
		std::vector<VehicleData> _vehicleDatas;

		//�ǻ���������
		std::mutex _bikeMutex;
		std::vector<BikeData> _bikeDatas;

		//���˶���
		std::mutex _pedestrainMutex;
		std::vector<PedestrainData> _pedestrainDatas;

		//ʱ�����
		std::mutex _eventMutex;
		std::vector<EventData> _eventDatas;
	};

}

