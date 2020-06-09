#pragma once
#include "EventData.h"
#include "TrafficDetector.h"
#include "EncodeHandler.h"
#include "ImageHandler.h"

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
		EventDetector(int width, int height,MqttChannel* mqtt, bool debug);

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

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer,const unsigned char* yuvBuffer,int frameIndex,int frameSpan);

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
				:LaneIndex(0), LaneType(EventLaneType::None), Region(), XTrend(false), YTrend(false),BaseAsX(false)
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
		class EventEncoderCache
		{
		public:
			EventEncoderCache(const std::string& channelUrl,int laneIndex,long long timeStamp,EventType eventType,const std::string& videoFilePath,int width,int height)
				:Json(),Encoder(&Json, videoFilePath, width, height, 10), Image(&Json,2,width,height)
			{
				JsonSerialization::SerializeValue(&Json, "channelUrl", channelUrl);
				JsonSerialization::SerializeValue(&Json, "laneIndex", laneIndex);
				JsonSerialization::SerializeValue(&Json, "timeStamp", timeStamp);
				JsonSerialization::SerializeValue(&Json, "type", (int)eventType);
			}
			//json
			std::string Json;
			//��Ƶ
			EncodeHandler Encoder;
			//ͼƬ
			ImageHandler Image;
		};

		/**
		* @brief: ���������¼�ͼƬ
		* @param: iveBuffer ive�ֽ���
		* @param: points ���㼯��
		* @param: frameIndex ֡���
		*/
		void DrawRetrograde(const unsigned char* iveBuffer, const std::vector<Point>& points, int frameIndex);
		
		/**
		* @brief: ���������¼�ͼƬ
		* @param: iveBuffer ive�ֽ���
		* @param: point ���˵�
		* @param: frameIndex ֡���
		*/
		void DrawPedestrain(const unsigned char* iveBuffer, const Point& point, int frameIndex);
		
		/**
		* @brief: ���������¼�ͼƬ
		* @param: iveBuffer ive�ֽ���
		* @param: point ͣ����
		* @param: frameIndex ֡���
		*/
		void DrawPark(const unsigned char* iveBuffer, const Point& point, int frameIndex);
		
		/**
		* @brief: ����ӵ������
		* @param: iveBuffer ive�ֽ���
		* @param: frameIndex ֡���
		*/
		void DrawCongestion(const unsigned char* iveBuffer, int frameIndex);

		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: iveBuffer ive�ֽ���
		* @param: frameIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex);

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
		//���ı������
		static const int MaxEncodeCount;

		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<EventLaneCache> _lanes;

		//��Ҫ������¼�����
		std::vector<EventEncoderCache*> _encoders;

		//yuv420p�ֽ���
		unsigned char* _yuv420pBuffer;

	};

}

