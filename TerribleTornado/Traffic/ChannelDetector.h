#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "SeemmoSDK.h"
#include "FFmpegChannel.h"
#include "JsonFormatter.h"
#include "LaneDetector.h"
#include "MqttChannel.h"
#include "BGR24Handler.h"
#include "JPGHandler.h"

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
		* @param: debug �Ƿ��ڵ���ģʽ�����ڵ���ģʽ��������ߺ��bmp
		*/
		ChannelDetector(int width, int height, MqttChannel* mqtt,bool debug=false);

		/**
		* @brief: ��������
		*/
		~ChannelDetector();

		/**
		* @brief: ��ȡͨ����ַ
		* @return ͨ����ַ
		*/
		std::string ChannelUrl() const;

		/**
		* @brief: ��ȡ�����Ƿ��ʼ���ɹ�
		* @return �����Ƿ��ʼ���ɹ�
		*/
		bool LanesInited() const;

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
		* @param: params ������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: packetIndex ֡���
		* @return: ʶ�����ݼ���
		*/
		std::vector<RecognItem> HandleDetect(const std::string& detectJson, std::string* param, const unsigned char* iveBuffer,long long packetIndex);
		
		/**
		* @brief: ����ʶ������
		* @param: item ʶ��������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: recognJson ʶ��json����
		*/
		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer,const std::string& recognJson);

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

		/**
		* @brief: hisi ive_8uc3תbgr
		* @param: iveBuffer ive�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: bgrBuffer д���bgr�ֽ���
		*/
		void IveToBgr(const unsigned char* iveBuffer,int width,int height,unsigned char* bgrBuffer);
		
		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: iveBuffer ive�ֽ���
		* @param: packetIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex);

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
		//������ʼ���Ƿ�ɹ�
		bool _lanesInited;
		//�����㷨ɸѡ�������
		std::string _param;
		//�Ƿ����ù���������
		bool _setParam;
		//bgr�ֽ���
		unsigned char* _bgrBuffer;
		//�Ƿ��ڵ���ģʽ
		bool _debug;
		//������bgr�ֽ���
		unsigned char* _debugBgrBuffer;
		//����ʱдbmp
		BGR24Handler _bgrHandler;
		//����ʱдjpg
		JPGHandler _jpgHandler;
	};

}

