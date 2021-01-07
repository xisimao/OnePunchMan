#pragma once
#include "TrafficData.h"
#include "ImageConvert.h"
#include "EncodeChannel.h"
#include "DataChannel.h"
#include "ImageDrawing.h"

namespace OnePunchMan
{
	//ͨ�����
	class EventDetector 
	{
	public:
		/**
		* ���캯��
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param encodeChannel �����߳�
		* @param dataChannel �����߳�
		*/
		EventDetector(int width, int height, EncodeChannel* encodeChannel, DataChannel* dataChannel);
		
		/**
		* ��������
		*/
		~EventDetector();

		/**
		* ��ʼ���¼���������
		* @param jd json����
		*/
		static void Init(const JsonDeserialization& jd);

		/**
		* ����ͨ��
		* @param taskId �����
		* @param channel ͨ��
		*/
		void UpdateChannel(const unsigned char taskId, const TrafficChannel& channel);

		/**
		* ���ͨ��
		*/
		void ClearChannel();

		/**
		* ��ȡ�����Ƿ��ʼ���ɹ�
		* @return �����Ƿ��ʼ���ɹ�
		*/
		bool LanesInited() const;

		/**
		* �¼����
		* @param items ����������
		* @param timeStamp ʱ���
		* @param taskId ������
		* @param iveBuffer ͼƬ�ֽ���
		* @param frameIndex ֡���
		* @param frameSpan ֡���ʱ��(����)
		*/
		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, unsigned char taskId, const unsigned char* iveBuffer,unsigned int frameIndex, unsigned char frameSpan);

	private:
		//�¼���⻺��
		class EventDetectCache
		{
		public:
			EventDetectCache()
				:FirstTimeStamp(0),LastTimeStamp(0), StartPark(false), StopPark(false),StartParkImage(), RetrogradePoints()
			{

			}
			//������һ�γ��ֵ�ʱ���,ͣ�����
			long long FirstTimeStamp;
			//��������һ�γ��ֵ�ʱ���,ɾ������,���˼��,ͣ�����
			long long LastTimeStamp;
			//�Ƿ��Ѿ���ʼ����ͣ��,ͣ�����
			bool StartPark;
			//�Ƿ��Ѿ���������ͣ��,ͣ�����
			bool StopPark;
			//��ʼͣ����ͼƬ,ͣ�����
			std::string StartParkImage;
			//���������еļ��㼯��,���м��
			std::vector<Point> RetrogradePoints;
		};

		//�¼���������
		class EventLaneCache
		{
		public:
			EventLaneCache()
				:LaneIndex(0), LaneId(),LaneType(EventLaneType::None), Region(), XTrend(true), YTrend(true), BaseAsX(false)
				, Congestion(false),LastReportTimeStamp(0), Items()
			{

			}

			//�������
			int LaneIndex;
			//�������
			std::string LaneId;
			//��������
			EventLaneType LaneType;
			//��ǰ�������
			Polygon Region;
			//x�ƶ�������,true��ʾ����,���м��
			bool XTrend;
			//y�ƶ�������,true��ʾ����,���м��
			bool YTrend;
			//�жϵ�����ϵ
			bool BaseAsX;

			//��ǰ�����Ƿ���ӵ��״̬
			bool Congestion;
			//��һ���ϱ���ʱ���,ӵ�¼��
			long long LastReportTimeStamp;

			//�����ڼ�����
			std::map<std::string, EventDetectCache> Items;

		};

		/**
		* ���Ƴ����¼�ͼƬ
		* @param filePath д���ļ�·��
		* @param iveBuffer ive�ֽ���
		* @param detectRegion ���������
		*/
		void DrawDetect(const std::string& filePath,const unsigned char* iveBuffer, const Rectangle& detectRegion);

		//IO mqtt����
		static const std::string EventTopic;
		//�����Ƴ���ʱ����(����)
		static const int DeleteSpan;
		//��໺��ļ���������
		static const int MaxCacheCount;

		//��ʼ����ͣ���¼���ʱ�䳤�ȣ���λ:����
		static int ParkStartSpan;
		//ȷ��ͣ���¼���ʱ�䳤�ȣ���λ:����
		static int ParkEndSpan;
		//�ж�ӵ��ʱ�������ڵ���С����������
		static int CongestionCarCount;
		//�ϱ�ӵ�µ�ʱ����,��λ:����
		static int CongestionReportSpan;
		//�жϳ������е����С���о��룬��λ������
		static double RetrogradeMinMove;
		//ȷ���������е���С���е�ĸ���
		static unsigned int RetrogradeMinCount;
		//�����Ƶ��I֡����
		static int OutputVideoIFrame;

		//��Ƶ���
		int _channelIndex;
		//��Ƶ��ַ
		std::string _channelUrl;
		//��Ƶ���
		int _width;
		//��Ƶ�߶�
		int _height;
		//������ʼ���Ƿ�ɹ�
		bool _lanesInited;

		//������
		int _taskId;

		//��������ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<EventLaneCache> _lanes;
		//�ȴ�����ļ���
		std::vector<EventData> _encodeDatas;
		//ͼ��ת��
		ImageConvert _image;
		//�����߳�
		EncodeChannel* _encodeChannel;
		//�¼������߳�
		DataChannel* _dataChannel;
		//��Ƶ�ļ���ʱ����ֽ�������
		int _videoSize;
		//��Ƶ�ļ���ʱ����ֽ���
		unsigned char* _videoBuffer;
	};

}

