#pragma once
#include <vector>

#include "SeemmoSDK.h"
#include "FFmpegChannel.h"
#include "JsonFormatter.h"
#include "LaneDetector.h"
#include "MqttChannel.h"
#include "IVE_8UC3Handler.h"
#include "BGR24Handler.h"

namespace OnePunchMan
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
		* @brief: ��ȡͨ����ַ
		* @return ͨ����ַ
		*/
		std::string ChannelUrl();

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
		* @param: detectJson ���json����
		* @param: bgrBuffer bgr�ֽ���
		* @param: params ������
		* @return: ʶ�����ݼ���
		*/
		std::vector<RecognItem> HandleDetect(const std::string& detectJson, unsigned char* bgrBuffer, std::string* param);
		
		/**
		* @brief: ����ʶ������
		* @param: item ʶ��������
		* @param: recognJson ʶ��json����
		*/
		void HandleRecognize(const RecognItem& item,uint8_t* bgrBuffer,const std::string& recognJson);

	private:
		//��ѯ��˯��ʱ��(����)
		static const int SleepTime;

		//IO mqtt����
		static const std::string IOTopic;
		//��Ƶ�ṹ��mqtt����
		static const std::string VideoStructTopic;

		/**
		* @brief: ��json�����л�ȡ������
		* @param: items ʶ�����
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: ��json�����л�ȡ������
		* @param: items �����
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		//��Ƶ���
		int _channelIndex;
		//��Ƶ��ַ
		std::string _channelUrl;
		//��Ƶ���
		int _width;
		//��Ƶ�߶�
		int _height;
		//mqtt
		MqttChannel* _mqtt;
		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<LaneDetector*> _lanes;
		//8uc3ת��
		IVE_8UC3Handler _handler;
		//bgr
		BGR24Handler _bgrHandler;
		//�����㷨ɸѡ�������
		std::string _param;
		//�Ƿ����ù���������
		bool _setParam;
		//��Ƶ��������
		std::string _videoTopic;
		//����ѹ��jpgͼƬ�Ĳ���
		std::vector<int> _jpgParams;
	};

}

