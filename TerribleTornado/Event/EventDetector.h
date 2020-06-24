#pragma once
#include "EventData.h"
#include "TrafficDetector.h"
#include "HisiEncodeChannel.h"

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
		* @param: encodeChannel ����ͨ��
		*/
		EventDetector(int width, int height,MqttChannel* mqtt, HisiEncodeChannel* encodeChannel);
		
		/**
		* @brief: ��������
		*/
		~EventDetector();

		/**
		* @brief: ����ͨ��
		* @param: channel ͨ��
		*/
		void UpdateChannel(const EventChannel& channel);

		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, unsigned char taskId, const unsigned char* iveBuffer,unsigned int frameIndex, unsigned char frameSpan);

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
				:LaneIndex(0), LaneType(EventLaneType::None), Region(), XTrend(true), YTrend(true), BaseAsX(false)
				, Congestion(false),LastReportTimeStamp(0), Items()
			{

			}

			//�������
			int LaneIndex;
			//��������
			EventLaneType LaneType;
			//��ǰ�������
			Polygon Region;
			//x�ƶ������ƣ�true��ʾ�������м��
			bool XTrend;
			//y�ƶ������ƣ�true��ʾ�������м��
			bool YTrend;
			//�жϵ�����ϵ
			bool BaseAsX;

			//��ǰ�����Ƿ���ӵ��״̬
			bool Congestion;
			//��һ���ϱ���ʱ�����ӵ�¼��
			long long LastReportTimeStamp;

			//�����ڼ�����
			std::map<std::string, EventDetectCache> Items;

		};

		//�¼����뻺��
		class EventEncodeCache
		{
		public:
			EventEncodeCache()
				:Json(), EncodeIndex(-1), FilePath()
			{

			}

			std::string Json;
			int EncodeIndex;
			std::string FilePath;
		};

		/**
		* @brief: ���������¼�ͼƬ
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: points ���㼯��
		* @param: frameIndex ֡���
		*/
		void DrawRetrograde(std::string* jpgBase64, const unsigned char* iveBuffer, const std::vector<Point>& points, unsigned int frameIndex);
		
		/**
		* @brief: ���������¼�ͼƬ
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: point ���˵�
		* @param: frameIndex ֡���
		*/
		void DrawPedestrain(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, unsigned int frameIndex);
		
		/**
		* @brief: ���������¼�ͼƬ
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: point ͣ����
		* @param: frameIndex ֡���
		*/
		void DrawPark(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, unsigned int frameIndex);
		
		/**
		* @brief: ����ӵ������
		* @param: jpgBase64 ����д����ַ���
		* @param: iveBuffer ive�ֽ���
		* @param: frameIndex ֡���
		*/
		void DrawCongestion(std::string* jpgBase64, const unsigned char* iveBuffer, unsigned int frameIndex);

		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: iveBuffer ive�ֽ���
		* @param: frameIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, unsigned int frameIndex);

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
		//�ȴ�����ļ���
		std::vector<EventEncodeCache> _encodes;
		//bgr�ֽ���
		unsigned char* _bgrBuffer;
		//jpg�ֽ���
		unsigned char* _jpgBuffer;

		HisiEncodeChannel* _encodeChannel;
		int _videoSize;
		unsigned char* _videoBuffer;
		
	};

}

