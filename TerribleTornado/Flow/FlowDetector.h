#pragma once
#include "FlowData.h"
#include "TrafficDetector.h"

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
		* @param: debug �Ƿ��ڵ���ģʽ�����ڵ���ģʽ��������ߺ��bmp
		*/
		FlowDetector(int width, int height, MqttChannel* mqtt, bool debug);

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

		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, int frameIndex,int frameSpan);

		void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer, const std::string& recognJson);

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
				, TotalDistance(0.0), TotalTime(0)
				, TotalInTime(0)
				, LastInRegion(0), Vehicles(0), TotalSpan(0)
				, IoStatus(false), Flag(false)
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

			//���ڼ���ʱ��ռ����
			//����ռ����ʱ��(����)
			long long TotalInTime;

			//���ڼ��㳵ͷʱ��
			//��һ���г����������ʱ��� 
			long long LastInRegion;
			//����������
			int Vehicles;
			//������������ʱ���ĺ�(����)
			long long TotalSpan;

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

		/**
		* @brief: ���Ƽ������
		* @param: detectItems ������
		* @param: iveBuffer ive�ֽ���
		* @param: frameIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex);

		//IO mqtt����
		static const std::string IOTopic;
		//��Ƶ�ṹ��mqtt����
		static const std::string VideoStructTopic;

		//��һ֡��ʱ���
		long long _lastTimeStamp;

		//��������ͬ����
		std::timed_mutex _laneMutex;
		//��������
		std::vector<FlowLaneCache> _lanes;
	};

}

