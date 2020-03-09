#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"

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
			:DetectItem(Rectangle(),std::string(), 0, 0)
		{

		}

		/**
		* @brief: ���캯��
		* @param: region ���Ԫ������
		*/
		DetectItem(const Rectangle& region)
			:DetectItem(region,std::string(),0,0)
		{

		}

		/**
		* @brief: ���캯��
		* @param: region ���Ԫ������
		* @param: id ���Ԫ�ر��
		* @param: timeStamp ʱ���
		* @param: type ���Ԫ������
		*/
		DetectItem(const Rectangle& region,const std::string& id, long long timeStamp, int type)
			:Region(region), Id(id), TimeStamp(timeStamp), Type(type)
		{

		}

		//���Ԫ�ر��
		std::string Id;
		//���Ԫ������
		int Type;
		//ʱ���
		long long TimeStamp;
		//���Ԫ������
		Rectangle Region;
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

	//��������������
	class LaneItem
	{
	public:

		//�������
		std::string Id;
		//�������
		int Index;

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
		//ʱ��ռ����(%)
		double TimeOccupancy;
	};

	class VideoStruct
	{
	public:
		std::string ChannelId;
		std::string LaneId;
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
		* @param: id �������
		* @param: index �������
		* @param: region ��������
		*/
		LaneDetector(const std::string& id,int index,Polygon region);
	
		/**
		* @brief: ��ȡ�������
		* @return: �������
		*/
		std::string Id();

		/**
		* @brief: ��ȡ�������
		* @return: �������
		*/
		int Index();

		/**
		* @brief: ��������
		* @param: item �����
		* @return: ���������ڳ����ڷ���true�����򷵻�false
		*/
		bool DetectVehicle(const DetectItem& item);

		/**
		* @brief: ���ǻ�����
		* @param: item �����
		* @return: ���������ڳ����ڷ���true�����򷵻�false
		*/
		bool DetectBike(const DetectItem& item);

		/**
		* @brief: �������
		* @param: item �����
		* @return: ���������ڳ����ڷ���true�����򷵻�false
		*/
		bool DetectPedestrain(const DetectItem& item);

		/**
		* @brief: ��������Ƿ��ڳ�����Χ��
		* @param: item �����
		* @return: �ڳ����ڷ���true
		*/
		bool Contains(const DetectItem& item);

		/**
		* @brief: �ռ�������������
		* @return: ������������
		*/
		LaneItem Collect();

		//����IO״̬
		bool Status;

	private:

		//ͬ����
		std::mutex _mutex;
		//��ǰ���������
		std::map<std::string, DetectItem> _items;

		//�������
		std::string _id;
		//�������
		int _index;
		//��ǰ�������
		Polygon _region;

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
	};

}

