#pragma once
#include <list>

#include "FlowData.h"
#include "TrafficDetector.h"
#include "Command.h"

namespace OnePunchMan
{
	//ͨ�����
	class FlowDetector :public TrafficDetector
	{
	public:
		/**
		* @brief: ���캯��
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: mqtt mqtt
		*/
		FlowDetector(int width, int height,MqttChannel* mqtt);
		
		/**
		* @brief: ��������
		*/
		~FlowDetector();

		/**
		* @brief: ����ͨ��
		* @param: taskId ������
		* @param: channel ͨ��
		*/
		void UpdateChannel(const unsigned char taskId,const FlowChannel& channel);

		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

		/**
		* @brief: ��ȡ����json����
		* @param: json ���ڴ��json���ݵ��ַ���
		*/
		void GetReportJson(std::string* json);

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex,unsigned char frameSpan);
		
		void FinishDetect(unsigned char taskId);

		void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle);
		
		void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike);
		
		void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain);

	private:
		//������⻺��
		class FlowDetectCache
		{
		public:
			FlowDetectCache()
				:LastTimeStamp(0),LastHitPoint()
			{

			}
			//��������һ�γ��ֵ�ʱ���
			long long LastTimeStamp;
			//��������һ�μ���
			Point LastHitPoint;
		};

		//������������
		class FlowLaneCache
		{
		public:
			FlowLaneCache()
				: LaneId(),LaneName(), Direction(), Region(),MeterPerPixel(0.0), StopPoint()
				, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
				, TotalDistance(0.0), TotalTime(0), Speed(0.0)
				, TotalInTime(0), TimeOccupancy(0.0)
				, LastInRegion(0), Vehicles(0), TotalSpan(0), HeadDistance(0.0),HeadSpace(0.0)
				, QueueLength(0)
				, TrafficStatus(0),IoStatus(false),Items()
			{

			}

			//�������
			std::string LaneId;
			//��������
			std::string LaneName;
			//��������
			int Direction;
			//��ǰ�������
			Polygon Region;
			//ÿ�����ش��������
			double MeterPerPixel;
			//ֹͣ�߼��ĵ�
			Point StopPoint;

			//��������
			int Persons;
			//���г�����
			int Bikes;
			//Ħ�г�����
			int Motorcycles;
			//�γ�����
			int Cars;
			//���ֳ�����
			int Tricycles;
			//����������
			int Buss;
			//���������
			int Vans;
			//��������
			int Trucks;

			//���ڼ���ƽ���ٶ�
			//������ʻ�ܾ���(����ֵ) 
			double TotalDistance;
			//������ʻ��ʱ��(����)
			long long TotalTime;
			//ƽ���ٶ�(km/h)
			double Speed;

			//���ڼ���ʱ��ռ����
			//����ռ����ʱ��(����)
			long long TotalInTime;
			//ʱ��ռ����(%)
			double TimeOccupancy;

			//���ڼ��㳵ͷʱ��
			//��һ���г����������ʱ��� 
			long long LastInRegion;
			//����������
			int Vehicles;
			//������������ʱ���ĺ�(����)
			long long TotalSpan;
			//����ʱ��(sec)
			double HeadDistance;
			//��ͷ���(m)
			double HeadSpace;

			//�Ŷӳ���(m)
			int QueueLength;

			//��ͨ״̬
			int TrafficStatus;

			//io״̬
			bool IoStatus;

			//�����ڼ�����
			std::map<std::string, FlowDetectCache> Items;
		};

		//�������滺��
		class FlowReportCache
		{
		public:
			FlowReportCache()
				: LaneId(), LaneName(), Direction(0), Minute(0)
				, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
				, Speed(0.0), TimeOccupancy(0.0), HeadDistance(0.0), HeadSpace(0.0), QueueLength(0), TrafficStatus(0)
			{

			}

			//�������
			std::string LaneId;
			//��������
			std::string LaneName;
			//��������
			int Direction;
			//�ڼ�����
			int Minute;
			//��������
			int Persons;
			//���г�����
			int Bikes;
			//Ħ�г�����
			int Motorcycles;
			//�γ�����
			int Cars;
			//���ֳ�����
			int Tricycles;
			//����������
			int Buss;
			//���������
			int Vans;
			//��������
			int Trucks;

			//ƽ���ٶ�(km/h)
			double Speed;
			//ʱ��ռ����(%)
			double TimeOccupancy;
			//����ʱ��(sec)
			double HeadDistance;
			//��ͷ���(m)
			double HeadSpace;
			//�Ŷӳ���(m)
			int QueueLength;
			//��ͨ״̬
			int TrafficStatus;
		};

		//�����ľ���
		class CarDistance
		{
		public:
			CarDistance()
				:Distance(0),Length(0)
			{

			}
			double Distance;
			int Length;
		};

		/**
		* @brief: �����������
		* @param: laneCache ��������
		*/
		void CalculateMinuteFlow(FlowLaneCache* laneCache);

		/**
		* @brief: ��ӳ������������б�
		* @param: distances ������������
		* @param: type ��������
		* @param: distance ��������ֹͣ�߳���(px)
		*/
		void AddOrderedList(std::list<CarDistance>* distances, DetectType type,double distance);

		/**
		* @brief: �����Ŷӳ���
		* @param: distances ������������
		*/
		int CalculateQueueLength(const std::list<CarDistance>& distances);

		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: ioChanged �Ƿ���io�仯��֡
		* @param: iveBuffer ive�ֽ���
		* @param: frameIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, unsigned int frameIndex);

		//IO mqtt����
		static const std::string IOTopic;
		//����mqtt����
		static const std::string FlowTopic;
		//��Ƶ�ṹ��mqtt����
		static const std::string VideoStructTopic;
		//�ϱ������ʱ��(����)
		static const int ReportMaxSpan;
		//�����Ƴ���ʱ����(����)
		static const int DeleteSpan;
		//�����Ŷ�ʱ��������С����(px)
		static const int MinCarDistance;
		//������
		int _taskId;

		//��һ֡��ʱ���
		long long _lastFrameTimeStamp;
		//��ǰ���ӵ�ʱ���
		long long _currentMinuteTimeStamp;
		//��һ���ӵ�ʱ���
		long long _nextMinuteTimeStamp;

		//��⳵������ͬ����
		std::mutex _laneMutex;
		//��⳵������
		std::vector<FlowLaneCache> _laneCaches;
		//�ϱ�����
		std::vector<FlowReportCache> _reportCaches;
		std::vector<VideoStruct_Vehicle> _vehicleReportCaches;
		std::vector<VideoStruct_Bike> _bikeReportCaches;
		std::vector<VideoStruct_Pedestrain> _pedestrainReportCaches;

		//���ʱ�õ���bgr�ֽ���
		unsigned char* _detectBgrBuffer;
		//���ʱ�õ���jpg�ֽ���
		unsigned char* _detectJpgBuffer;
		//ʶ��ʱ�õ���bgr�ֽ���
		unsigned char* _recognBgrBuffer;
		//ʶ��ʱ�õ���jpg�ֽ���
		unsigned char* _recognJpgBuffer;

		//�Ƿ����ͼƬ
		bool _outputImage;
		//�Ƿ������ⱨ��
		bool _outputReport;
		//��ǰ����ķ���
		int _currentReportMinute;
		//�Ƿ����ʶ����
		bool _outputRecogn;
	};

}

