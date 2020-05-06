#pragma once
#include <vector>

#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

#include "ChannelDetector.h"
#include "LaneDetector.h"
#include "BGR24Handler.h"
#include "JPGHandler.h"

namespace OnePunchMan
{
	//ͨ�����
	class FlowChannelDetector:public ChannelDetector
	{
	public:
		/**
		* @brief: ���캯��
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: mqtt mqtt
		* @param: debug �Ƿ��ڵ���ģʽ�����ڵ���ģʽ��������ߺ��bmp
		*/
		FlowChannelDetector(int width, int height, MqttChannel* mqtt,bool debug=false);

		/**
		* @brief: ��������
		*/
		~FlowChannelDetector();

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
		* @brief: ����ʶ������
		* @param: item ʶ��������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: recognJson ʶ��json����
		*/
		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer,const std::string& recognJson);
	
	protected:
		/**
		* @brief: ������ʵ�ֵĴ���������
		* @param: items ����������
		* @param: timeStamp ʱ���
		*/
		void HandleDetectCore(std::map<std::string, DetectItem> detectItems, long long timeStamp, const unsigned char* iveBuffer, long long packetIndex);

	private:
		//IO mqtt����
		static const std::string IOTopic;
		//��Ƶ�ṹ��mqtt����
		static const std::string VideoStructTopic;

		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: iveBuffer ive�ֽ���
		* @param: packetIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex);

		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<LaneDetector*> _lanes;
		//������ʼ���Ƿ�ɹ�
		bool _lanesInited;
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

