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
		* @param: width ͼƬ����
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

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer,int packetIndex,int frameSpan);

		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer, const std::string& recognJson);

	private:

		//�¼���⻺��
		class EventDetectCache
		{
		public:
			EventDetectCache()
				:FirstTimeStamp(0),LastTimeStamp(0), StartPark(false), StopPark(false),StartParkImage(), RetrogradePoints()
			{

			}
			//������һ�γ��ֵ�ʱ�����ͣ�����
			long long FirstTimeStamp;
			//��������һ�γ��ֵ�ʱ�����ɾ�����棬���˼�⣬ͣ�����
			long long LastTimeStamp;
			//�Ƿ��Ѿ���ʼ����ͣ����ͣ�����
			bool StartPark;
			//�Ƿ��Ѿ���������ͣ����ͣ�����
			bool StopPark;
			//��ʼͣ����ͼƬ��ͣ�����
			std::string StartParkImage;
			//���������еļ��㼯�ϣ����м��
			std::vector<Point> RetrogradePoints;
		};

		//�¼���������
		class EventLaneCache
		{
		public:
			EventLaneCache()
				:LaneId(), LaneType(EventLaneType::None), Region(), XTrend(true), YTrend(true)
				, Congestion(false),LastReportTimeStamp(0), Items()
			{

			}

			//�������
			std::string LaneId;
			//��������
			EventLaneType LaneType;
			//��ǰ�������
			Polygon Region;
			//x�ƶ������ƣ�true��ʾ�������м��
			bool XTrend;
			//y�ƶ������ƣ�true��ʾ�������м��
			bool YTrend;

			//��ǰ�����Ƿ���ӵ��״̬
			bool Congestion;
			//��һ���ϱ���ʱ�����ӵ�¼��
			long long LastReportTimeStamp;

			//�����ڼ�����
			std::map<std::string, EventDetectCache> Items;

		};

		/**
		* @brief: ���������¼�ͼƬ
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: points ���㼯��
		* @param: packetIndex ֡���
		*/
		void DrawRetrograde(std::string* jpgBase64, const unsigned char* iveBuffer, const std::vector<Point>& points, int packetIndex);
		
		/**
		* @brief: ���������¼�ͼƬ
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: point ���˵�
		* @param: packetIndex ֡���
		*/
		void DrawPedestrain(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, int packetIndex);
		
		/**
		* @brief: ���������¼�ͼƬ
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: point ͣ����
		* @param: packetIndex ֡���
		*/
		void DrawPark(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, int packetIndex);
		
		/**
		* @brief: ����ӵ������
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: packetIndex ֡���
		*/
		void DrawCongestion(std::string* jpgBase64, const unsigned char* iveBuffer, int packetIndex);

		//IO mqtt����
		static const std::string EventTopic;
		//�����Ƴ���ʱ����(����)
		static const int DeleteSpan;
		//ͣ����ʼ�����ʱ����(����)
		static const int ParkStartSpan;
		//ͣ�����������ʱ����(����)
		static const int ParkEndSpan;
		//�����ж�ӵ�µĳ�����
		static const int CarCount;
		//�ϱ�ӵ���¼����(����)
		static const int ReportSpan;
		//���м���õ����ƶ�����
		static const double MovePixel;
		//�ж������¼��ĵ������
		static const int PointCount;

		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<EventLaneCache> _lanes;

	};

}
