#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "ChannelDetector.h"
#include "EventDetector.h"

namespace OnePunchMan
{
	//ͨ�����
	class EventChannelDetector:public ChannelDetector
	{
	public:
		/**
		* @brief: ���캯��
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: mqtt mqtt
		*/
		EventChannelDetector(int width, int height, MqttChannel* mqtt);

		/**
		* @brief: ��������
		*/
		~EventChannelDetector();

		/**
		* @brief: ����ͨ��
		* @param: channel ͨ��
		*/
		void UpdateChannel(const EventChannel& channel);

		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

	protected:
		/**
		* @brief: ������ʵ�ֵĴ���������
		* @param: items ����������
		* @param: timeStamp ʱ���
		*/
		void HandleDetectCore(std::map<std::string, DetectItem> detectItems,long long timeStamp);

	private:
		//�¼� mqtt����
		static const std::string EventTopic;

		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<EventDetector*> _lanes;
		
	};

}

