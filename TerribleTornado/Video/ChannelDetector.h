#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "SeemmoSDK.h"
#include "FFmpegChannel.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "TrafficData.h"

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
		* @brief: ��������
		*/
		virtual ~ChannelDetector();

		/**
		* @brief: ��ȡͨ����ַ
		* @return ͨ����ַ
		*/
		std::string ChannelUrl() const;

		/**
		* @brief: ����������
		* @param: detectJson ���json����
		* @param: params ������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: packetIndex ֡���
		* @return: ʶ�����ݼ���
		*/
		std::vector<RecognItem> HandleDetect(const std::string& detectJson, std::string* param, const unsigned char* iveBuffer, long long packetIndex);

		/**
		* @brief: ����ʶ������
		* @param: item ʶ��������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: recognJson ʶ��json����
		*/
		virtual void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer, const std::string& recognJson)=0;

	protected:
		/**
		* @brief: ������ʵ�ֵĴ���������
		* @param: items ����������
		* @param: timeStamp ʱ���
		*/
		virtual void HandleDetectCore(std::map<std::string, DetectItem> detectItems, long long timeStamp, const unsigned char* iveBuffer, long long packetIndex)=0;


		/**
		* @brief: hisi ive_8uc3תbgr
		* @param: iveBuffer ive�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: bgrBuffer д���bgr�ֽ���
		*/
		void IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer);
		
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
		//�����㷨ɸѡ�������
		std::string _param;
		//�Ƿ����ù���������
		bool _setParam;
		//bgr�ֽ���
		unsigned char* _bgrBuffer;

	private:
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
	};

}

