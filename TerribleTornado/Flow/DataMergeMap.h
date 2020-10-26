#pragma once
#include <map>
#include <string>
#include <mutex>

#include "Shape.h"
#include "LogPool.h"
#include "MqttChannel.h"
#include "FlowData.h"

namespace OnePunchMan
{
	//���ݺϲ��ֵ�
	class DataMergeMap
	{
	public:
		/**
		* ���캯��
		* @param mqtt mqtt
		*/
		DataMergeMap(MqttChannel* mqtt);

		/**
		* ������������
		* @param data ��������
		*/
		void PushData(const FlowReportData& data);

	private:
		//����mqtt����
		static const std::string FlowTopic;

		//mqtt
		MqttChannel* _mqtt;

		//ͬ����
		std::mutex _mutex;
		//���������ֵ�
		std::map<std::string, FlowReportData> _datas;
	};

}


