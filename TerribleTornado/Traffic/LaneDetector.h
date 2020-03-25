#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"
#include "FlowChannelData.h"

namespace Saitama
{
	//�����
	class DetectItem
	{
	public:

		/**
		* @brief: ���캯��
		*/
		DetectItem()
			:DetectItem(Rectangle(), 0)
		{

		}

		/**
		* @brief: ���캯��
		* @param: region ���Ԫ������
		*/
		DetectItem(const Rectangle& region)
			:DetectItem(region,0)
		{

		}

		/**
		* @brief: ���캯��
		* @param: region ���Ԫ������
		* @param: type ���Ԫ������
		*/
		DetectItem(const Rectangle& region,int type)
			:HitPoint(region.Top.X + region.Width / 2, region.Top.Y + region.Height), Type(type)
		{

		}

		//����
		Point HitPoint;
		//���Ԫ������
		int Type;
		
	};

	//���Ԫ������
	enum class DetectType
	{
		Pedestrain = 1,
		Bike = 2,
		Motobike = 3,
		Car = 4,
		Tricycle = 5,
		Bus = 6,
		Van = 7,
		Truck = 8
	};

	//io״̬
	enum class IOStatus
	{
		In = 1,
		Out = 0,
		UnChanged=-1
	};

	//��������������
	class LaneItem
	{
	public:

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
	};

	//��Ƶ�ṹ��
	class VideoStruct
	{
	public:
		std::string Image;
		std::string Feature;
		int VideoStructType;
	};

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

	class VideoBike :public VideoStruct
	{
	public:
		VideoBike()
		{
			VideoStructType = 2;
		}
		int BikeType;
	};

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
		* @param: dataId ���ݱ��
		* @param: lane ����
		*/
		LaneDetector(const std::string& dataId,const Lane& lane);

		/**
		* @brief: ��ȡ�������ݱ��
		* @return: �������ݱ��io״̬
		*/
		std::string DataId();

		/**
		* @brief: ��������
		* @param: item �����
		* @return: ���س���io״̬
		*/
		IOStatus Detect(const std::map<std::string, DetectItem>& items,long long timeStamp);

		/**
		* @brief: ��������Ƿ��ڳ�����Χ��
		* @param: item �����
		* @return: �ڳ����ڷ���true
		*/
		bool Contains(const DetectItem& item);

		/**
		* @brief: �ռ�������������
		* @param: item ����ʱ���
		* @return: ������������
		*/
		LaneItem Collect(long long timeStamp);

	private:

		/**
		* @brief: ��ʼ������
		* @param: lane ����
		*/
		void InitLane(const Lane& lane);

		/**
		* @brief: �����߶��ַ���
		* @param: lane �������ַ���
		* @return: �߶�
		*/
		Line GetLine(const std::string& line);

		//���ݱ��
		std::string _dataId;
		//��ǰ�������
		Polygon _region;
		//ÿ�����ش��������
		double _meterPerPixel;

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

		//�ܾ���(����ֵ)
		double _totalDistance;
		//��ʱ��(����)
		long long _totalTime;
		//��һ���г����������ʱ���
		long long _lastInRegion;
		//����������
		int _vehicles;
		//������������ʱ���ĺ�(����)
		long long _totalSpan;

		//io״̬
		IOStatus _iOStatus;

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

