#pragma once
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "DecodeChannel.h"
#include "RecognChannel.h"
#include "TrafficDetector.h"

namespace OnePunchMan
{
	//����߳�
	class DetectChannel :public ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: detectIndex ������
		* @param: width ��Ƶ�������
		* @param: height ��Ƶ�����߶�
		*/
		DetectChannel(int detectIndex,int width, int height);

		/**
		* @brief: ��ǰ����߳��Ƿ�δ��ʼ��
		* @return: ���δ��ʼ�����ߵ�ǰ�߳����ڽ��м�ⷵ��true�����򷵻�false
		*/
		bool Inited();

		/**
		* @brief: �����Ҫ����ͨ�����
		* @param: channelIndex ͨ�����
		*/
		void SetRecogn(RecognChannel* recogn);

		/**
		* @brief: �����Ҫ����ͨ�����
		* @param: channelIndex ͨ�����
		*/
		void AddChannel(int channelIndex,DecodeChannel* decode,TrafficDetector* detector);

	protected:
		void StartCore();

	private:

		//��Ƶ֡����
		class ChannelItem
		{
		public:
			ChannelItem()
				: ChannelIndex(0), Param(), Decode(NULL),Detector(NULL)
			{

			}
			//ͨ�����
			int ChannelIndex;
			//��������
			std::string Param;
			//����
			DecodeChannel* Decode;
			//���
			TrafficDetector* Detector;
		};

	private:
		/**
		* @brief: ��json�����л�ȡ������
		* @param: items ������
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* @brief: ��json�����л�ȡʶ�����
		* @param: items ʶ�����
		* @param: jd json����
		* @param: key ������ͣ����������ǻ����ͺ�����
		* @param: channelIndex ͨ�����
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key,int channelIndex);

		//��ѯ��˯��ʱ��(����)
		static const int SleepTime;
		//�������
		static const std::string DetectTopic;

		//�߳��Ƿ��ʼ�����
		bool _inited;
		//������
		int _detectIndex;
		//ͼƬ���
		int _width;
		//ͼƬ�߶�
		int _height;
		//��Ƶ֡����
		std::vector<ChannelItem> _channelItems;
		//����߳�
		RecognChannel* _recogn;

		//detect
		std::vector<uint8_t*> _ives;
		std::vector<int> _indexes;
		std::vector<uint64_t> _timeStamps;
		std::vector<uint32_t> _widths;
		std::vector<uint32_t> _heights;
		std::vector<const char*> _params;
		std::vector<char> _result;

	};

}

