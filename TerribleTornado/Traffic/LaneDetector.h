#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "LogPool.h"
#include "JsonFormatter.h"
#include "Observable.h"
#include "FlowChannel.h"

namespace OnePunchMan
{
	//���Ԫ��״̬
	enum class DetectStatus
	{
		New,
		In,
		Out
	};

	//���Ԫ������
	enum class DetectType
	{
		None=0,
		Pedestrain = 1,
		Bike = 2,
		Motobike = 3,
		Car = 4,
		Tricycle = 5,
		Bus = 6,
		Van = 7,
		Truck = 8
	};

	//�����
	class DetectItem
	{
	public:
		//����
		//�������
		Rectangle Region;
		//���Ԫ������
		DetectType Type;
		
		//���
		//���Ԫ��״̬
		DetectStatus Status;
		//�ƶ�����
		double Distance;
	};
	
	//ʶ����
	class RecognItem
	{
	public:
		//ͨ�����
		int ChannelIndex;
		//guid
		std::string Guid;
		//���������
		int Type;
		//�������
		Rectangle Region;
		//���
		int Width;
		//�߶�
		int Height;
	};

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
	class IOItem
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
	class FlowItem
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
		std::string Image;
		std::string Feature;
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
		* @param: laneId �������
		* @param: lane ����
		*/
		LaneDetector(const std::string& laneId,const Lane& lane);

		/**
		* @brief: ��������
		* @param: item ���������
		* @return: ���س���io״̬
		*/
		IOItem Detect(std::map<std::string, DetectItem>* items,long long timeStamp);

		/**
		* @brief: ʶ�������
		* @param: item ʶ��������
		* @return: ���ʶ�����ڳ����ڷ��س�����ŷ��򷵻ؿ��ַ���
		*/
		std::string Recogn(const RecognItem& item);

		/**
		* @brief: �ռ�������������
		* @param: item ����ʱ���
		* @return: ������������
		*/
		FlowItem Collect(long long timeStamp);
		
		/**
		* @brief: ��ȡ��ǰ�������
		* @return: ��ǰ�������
		*/
		const Polygon& Region();

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
		
		/**
		* @brief: ����������ַ���
		* @param: region �����ַ���
		* @return: �����
		*/
		Polygon GetPolygon(const std::string& region);
		
		//�������
		std::string _laneId;
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

