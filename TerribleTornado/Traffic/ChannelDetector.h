#pragma once
#include <vector>

#include "SeemmoSDK.h"
#include "FrameChannel.h"
#include "JsonFormatter.h"
#include "LaneDetector.h"
#include "MqttChannel.h"

namespace TerribleTornado
{
	//ͨ�����
	class ChannelDetector
	{
	public:
		/**
		* @brief: ���캯��
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: mqtt mqtt
		*/
		ChannelDetector(int width, int height, MqttChannel* mqtt);

		/**
		* @brief: ����ͨ��
		* @param: channel ͨ��
		*/
		void UpdateChannel(const FlowChannel& channel);
		
		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

		/**
		* @brief: �ռ�����
		* @param: flowJson ����json����
		* @param: timeStamp ʱ���
		* @return: ����json����
		*/
		void CollectFlow(std::string* flowJson, long long timeStamp);

		/**
		* @brief: ����������
		* @param: detectJson ���ṹ
		* @param: timeStamp ʱ���
		* @return: io json����
		*/
		std::vector<std::string> HandleDetect(const std::string& detectJson, long long timeStamp);
		
		/**
		* @brief: ����ʶ������
		* @param: recognJson ʶ������
		*/
		void HandleRecognize(const std::string& recognJson);

	private:
		//��ѯ��˯��ʱ��(����)
		static const int SleepTime;

		//IO mqtt����
		static const std::string IOTopic;
		//��Ƶ�ṹ��mqtt����
		static const std::string VideoStructTopic;

		/**
		* @brief: ��json�����л�ȡ������
		* @param: guids guid����
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		*/
		static void GetRecognGuids(std::vector<std::string>* guids, const Saitama::JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: ��json�����л�ȡ������
		* @param: items �����
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		*/
		static void GetDetecItems(std::map<std::string, DetectItem>* items, const Saitama::JsonDeserialization& jd, const std::string& key);

		//��Ƶ��ַ
		std::string _channelUrl;
		//mqtt
		MqttChannel* _mqtt;
		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<LaneDetector*> _lanes;
	};

}

