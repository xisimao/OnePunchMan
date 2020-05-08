#pragma once
#include "EventData.h"
#include "TrafficDetector.h"

namespace OnePunchMan
{
	//�¼�����
	enum class EventType
	{
		None = 0,
		Pedestrain = 1,
		Park = 2,
		Congestion = 3,
		Retrograde = 4
	};

	//ͨ�����
	class EventDetector :public TrafficDetector
	{
	public:
		/**
		* @brief: ���캯��
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: mqtt mqtt
		* @param: debug �Ƿ��ڵ���ģʽ�����ڵ���ģʽ��������ߺ��bmp
		*/
		EventDetector(int width, int height, MqttChannel* mqtt, bool debug);

		/**
		* @brief: ����ͨ��
		* @param: channel ͨ��
		*/
		void UpdateChannel(const EventChannel& channel);

		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, long long packetIndex);

		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer, const std::string& recognJson);

	private:

		//�¼���⻺��
		class EventDetectCache
		{
		public:
			EventDetectCache()
				:FirstTimeStamp(0),LastTimeStamp(0), StartPark(false), StopPark(false),StartParkImage(), StopRetrograde(false), HitPoint()
			{

			}
			//������һ�γ��ֵ�ʱ�����ͣ�����
			long long FirstTimeStamp;
			//�����ڶ��γ��ֵ�ʱ�����ɾ�����棬���˼�⣬ͣ�����
			long long LastTimeStamp;
			//�Ƿ�ʼ����ͣ����ͣ�����
			bool StartPark;
			//�Ƿ�ʼ����ͣ����ͣ�����
			bool StopPark;
			//��ʼͣ����ͼƬ��ͣ�����
			std::string StartParkImage;
			//ֹͣ�������
			bool StopRetrograde;
			//���㣬���м��
			Point HitPoint;
		};

		//�¼���������
		class EventLaneCache
		{
		public:
			EventLaneCache()
				:LaneId(), LaneType(EventLaneType::None), Region(), Items(), LastReportTimeStamp(0), XTrend(true), YTrend(true)
			{

			}

			//�������
			std::string LaneId;
			//��������
			EventLaneType LaneType;
			//��ǰ�������
			Polygon Region;
			//�����ڼ�����
			std::map<std::string, EventDetectCache> Items;
			//��һ���ϱ���ʱ�����ӵ�¼��
			long long LastReportTimeStamp;
			//x�ƶ������ƣ�true��ʾ�������м��
			bool XTrend;
			//y�ƶ������ƣ�true��ʾ�������м��
			bool YTrend;
		};

		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: iveBuffer ive�ֽ���
		* @param: packetIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex);
		
		/**
		* @brief: ��ȡ��ǰ֡��jpg base64�ַ���
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: packetIndex ֡���
		*/
		void GetJpgBase64(std::string* jpgBase64,const unsigned char* iveBuffer, long long packetIndex);

		//IO mqtt����
		static const std::string EventTopic;
		//�����Ƴ���ʱ����(����)
		static const int DeleteSpan;
		//ͣ����ʼ�����ʱ����(����)
		static const int ParkStartSpan;
		//ͣ�����������ʱ����(����)
		static const int ParkEndSpan;
		//�����ж�ӵ�µĳ�����
		static const int CarsCount;
		//�ϱ�ӵ���¼����(����)
		static const int ReportSpan;

		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<EventLaneCache> _lanes;

	};

}

