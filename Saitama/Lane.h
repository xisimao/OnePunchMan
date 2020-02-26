#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"

namespace Saitama
{
	class DetectItem
	{
	public:
		DetectItem()
			:DetectItem(std::string(),0,Rectangle())
		{

		}

		DetectItem(const std::string& id,int timeStamp,const Rectangle& region)
			:Id(id), TimeStamp(timeStamp), Region(region)
		{

		}

		std::string Id;
		long long TimeStamp;
		Rectangle Region;
	};

	enum class DetectType
	{
		Pedestrain=1,
		Bike =2,
		Motobike =3,
		Car =4,
		Tricycle =5,
		Bus =6,
		Van =7,
		Truck =8
	};

	class TrafficItem
	{
	public:

		int Bikes;
		int Tricycles;
		int Persons;
		int Cars;
		int Motorcycles;
		int Buss;
		int Trucks;
		int Vans;
		double Speed;
		double HeadDistance;
		double TimeOccupancy;
	};

	class Lane
	{
	public:

		Lane();

		Lane(Polygon region);

		void CollectVehicle(const Rectangle& detectRegion, long long timeStamp, const std::string& message);

		void CollectBike(const Rectangle& detectRegion, long long timeStamp, const std::string& message);

		void CollectPedestrain(const Rectangle& detectRegion,long long timeStamp,const std::string& message);

		TrafficItem Calculate();

	private:

		//ͬ����
		std::mutex _mutex;
		//��ǰ���������
		std::map<std::string, DetectItem> _items;
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

