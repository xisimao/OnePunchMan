#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"
#include "FlowData.h"

namespace OnePunchMan
{
	//��ͨ״̬
	enum class TrafficStatus
	{
		Good = 1,
		Normal = 2,
		Warning = 3,
		Bad = 4,
		Dead = 5
	};

	//io������
	class IOResult
	{
	public:
		//�������
		std::string LaneId;
		//�������
		DetectType Type;
		//io״̬
		bool Status;
		//io״̬�Ƿ�ı�
		bool Changed;
	};

	//����������
	class FlowResult
	{
	public:
		//�������
		std::string LaneId;

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
		//�ͳ�����
		int Buss;
		//���������
		int Vans;
		//��������
		int Trucks;
	
		//ƽ���ٶ�(ǧ��/Сʱ)
		double Speed;
		//��ͷʱ��(��)
		double HeadDistance;
		//��ͷ���(��)=ƽ���ٶ�*��ͷʱ��
		double HeadSpace;
		//ʱ��ռ����(%)
		double TimeOccupancy;		
		//��ͨ״̬
		int TrafficStatus;
	};

	//��Ƶ�ṹ��
	class VideoStruct
	{
	public:
		int VideoStructType;
	};

	//������
	class VideoVehicle :public VideoStruct
	{
	public:

		VideoVehicle()
		{
			VideoStructType = 1;
		}
		int CarType;
		int CarColor;
		std::string CarBrand;
		int PlateType;
		std::string PlateNumber;
	};

	//�ǻ�����
	class VideoBike :public VideoStruct
	{
	public:
		VideoBike()
		{
			VideoStructType = 2;
		}
		int BikeType;
	};

	//����
	class VideoPedestrain :public VideoStruct
	{
	public:
		VideoPedestrain()
		{
			VideoStructType = 3;
		}
		int Sex;
		int Age;
		int UpperColor;
	};

	//�������ݼ���
	class LaneDetector
	{
	public:

		/**
		* @brief: ���캯��
		* @param: lane ����
		*/
		LaneDetector(const FlowLane& lane);

		/**
		* @brief: ��ȡ�����Ƿ��ʼ���ɹ�
		* @return: �����Ƿ��ʼ���ɹ�
		*/
		bool Inited() const;

		/**
		* @brief: ���
		* @param: items ����������
		* @param: timeStamp ʱ���
		* @return: ���س���io״̬
		*/
		IOResult Detect(std::map<std::string, DetectItem>* items,long long timeStamp);

		/**
		* @brief: ʶ�������
		* @param: item ʶ��������
		* @return: ���ʶ�����ڳ����ڷ��س�����ŷ��򷵻ؿ��ַ���
		*/
		std::string Recogn(const RecognItem& item);

		/**
		* @brief: �ռ�������������
		* @param: timeStamp ����ʱ���
		* @return: ������������
		*/
		FlowResult Collect(long long timeStamp);
		
		/**
		* @brief: ��ȡ��ǰ�������
		* @return: ��ǰ�������
		*/
		const Polygon& Region();

	private:

		//�������
		std::string _laneId;
		//��ǰ�������
		Polygon _region;
		//ÿ�����ش��������
		double _meterPerPixel;
		//�����Ƿ��ʼ���ɹ�
		bool _inited;

		//��������
		int _persons;
		//���г�����
		int _bikes;
		//Ħ�г�����
		int _motorcycles;
		//�γ�����
		int _cars;
		//���ֳ�����
		int _tricycles;
		//����������
		int _buss;
		//���������
		int _vans;
		//��������
		int _trucks;

		//���ڼ���ƽ���ٶ�
		//������ʻ�ܾ���(����ֵ) 
		double _totalDistance;
		//������ʻ��ʱ��(����)
		long long _totalTime;

		//���ڼ���ʱ��ռ����
		//����ռ����ʱ��(����)
		long long _totalInTime;

		//���ڼ��㳵ͷʱ��
		//��һ���г����������ʱ��� 
		long long _lastInRegion;
		//����������
		int _vehicles;
		//������������ʱ���ĺ�(����)
		long long _totalSpan;

		//io״̬
		bool _iOStatus;

		//��һ֡��ʱ���
		long long _lastTimeStamp;
		//ͬ����
		std::mutex _mutex;
		std::map<std::string, DetectItem> _items1;
		std::map<std::string, DetectItem> _items2;
		std::map<std::string, DetectItem>* _currentItems;
		std::map<std::string, DetectItem>* _lastItems;

	};

}

