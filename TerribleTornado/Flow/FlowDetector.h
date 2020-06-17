#pragma once
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
		* @param: channel ͨ��
		*/
		void UpdateChannel(const FlowChannel& channel);

		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, int frameIndex,int frameSpan);
		
		void FinishDetect();

		void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle);
		
		void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike);
		
		void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain);

	private:
		//������⻺��
		class FlowDetectCache
		{
		public:
			FlowDetectCache()
				:HitPoint()
			{

			}

			//����
			Point HitPoint;
		};

		//������������
		class FlowLaneCache
		{
		public:
			FlowLaneCache()
				: LaneId(), Region(),MeterPerPixel(0.0)
				, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
				, TotalDistance(0.0), TotalTime(0), Speed(0.0)
				, TotalInTime(0), TimeOccupancy(0.0)
				, LastInRegion(0), Vehicles(0), TotalSpan(0), HeadDistance(0.0),HeadSpace(0.0)
				, TrafficStatus(0),IoStatus(false), Flag(false)
			{

			}

			//�������
			std::string LaneId;
			//��ǰ�������
			Polygon Region;
			//ÿ�����ش��������
			double MeterPerPixel;

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

			//��ͨ״̬
			int TrafficStatus;

			//io״̬
			bool IoStatus;

			//ָ��ָ��ʵ��λ�õĽ���仯��־
			bool Flag;
			//�����ڼ�����
			std::map<std::string, FlowDetectCache> Items1;
			std::map<std::string, FlowDetectCache> Items2;

			/**
			* @brief: ��ȡ���μ����ļ���ָ��
			* @return: ���μ����ļ���ָ��
			*/
			std::map<std::string, FlowDetectCache>* CurrentItems()
			{
				return Flag ? &Items1 : &Items2;
			}

			/**
			* @brief: ��ȡ��һ�μ����ļ���ָ��
			* @return: ��һ�μ����ļ���ָ��
			*/
			std::map<std::string, FlowDetectCache>* LastItems()
			{
				return Flag ? &Items2 : &Items1;
			}

			/**
			* @brief: ������ǰ����һ�ε�ָ��
			*/
			void SwitchFlag()
			{
				Flag = !Flag;
			}
		};

		//��ⱨ��д��
		class ReportWriter
		{
		public:
			ReportWriter(int channelIndex,int laneCount)
				:_laneCount(laneCount), _minute(0)
			{
				_file.open(StringEx::Combine("../logs/report_", channelIndex, ".txt"), std::ofstream::out);
				_file << std::setiosflags(std::ios::left) << std::setw(10) << "����"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "����"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "�γ�"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "����"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "�ͳ�"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "�����"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "���ֳ�"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "���г�"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "Ħ�г�"
					<< std::setiosflags(std::ios::left) << std::setw(10) << "����"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "ƽ���ٶ�(km/h)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "��ͷʱ��(sec)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "��ͷ���(m)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "ʱ��ռ����(%)"
					<< std::setiosflags(std::ios::left) << std::setw(15) << "��ͨ״̬(1-5)"
					<< std::endl;
				_file.flush();
			}

			~ReportWriter()
			{
				if (_file.is_open())
				{
					_file.close();
				}
			}

			void Write(const FlowLaneCache& cache)
			{
				_file << std::setiosflags(std::ios::left) << std::setw(10) << _minute/_laneCount+1
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.LaneId
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Cars
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Trucks
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Buss
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Vans
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Tricycles
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Bikes
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Motorcycles
					<< std::setiosflags(std::ios::left) << std::setw(10) << cache.Persons
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.Speed
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.HeadDistance
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.HeadSpace
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.TimeOccupancy
					<< std::setiosflags(std::ios::left) << std::setw(15) << cache.TrafficStatus
					<< std::endl;
				_file.flush();
				_minute += 1;
			}

		private:
			//�ļ��� 
			std::ofstream _file;
			//��������
			int _laneCount;
			//����
			int _minute;
		};

		/**
		* @brief: �����������
		* @param: laneCache ��������
		*/
		void CalculateMinuteFlow(FlowLaneCache* laneCache);

		/**
		* @brief: �жϼ�������ڳ���������г�����������
		* @param: json json�ַ���
		* @param: recognItem ʶ����
		* @param: iveBuffer ive�ֽ���
		*/
		void HandleRecogn(std::string* json,const RecognItem& recognItem, const unsigned char* iveBuffer);

		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: iveBuffer ive�ֽ���
		* @param: frameIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex);

		//IO mqtt����
		static const std::string IOTopic;
		//����mqtt����
		static const std::string FlowTopic;
		//��Ƶ�ṹ��mqtt����
		static const std::string VideoStructTopic;
		//�ϱ������ʱ��(����)
		static const int ReportMaxSpan;

		//��һ֡��ʱ���
		long long _lastFrameTimeStamp;

		//��ǰ���ӵ�ʱ���
		long long _currentMinuteTimeStamp;
		//��һ���ӵ�ʱ���
		long long _nextMinuteTimeStamp;

		//��⳵������ͬ����
		std::mutex _detectLaneMutex;
		//��⳵������
		std::vector<FlowLaneCache> _detectLanes;
		//ʶ�𳵵�����ͬ����
		std::mutex _recognLaneMutex;
		//ͨ����ַ
		std::string _recognChannelUrl;
		//ʶ�𳵵�����
		std::vector<FlowLaneCache> _recognLanes;

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
		//���������ⱨ��
		ReportWriter* _report;
	};

}

