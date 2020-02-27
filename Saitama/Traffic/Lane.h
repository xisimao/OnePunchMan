#pragma once
#include <map>
#include <string>
#include <vector>

#include "Shape.h"
#include "Detect.h"
#include "LogPool.h"
#include "JsonFormatter.h"

namespace Saitama
{
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
		//ʱ��ռ����(%)
		double TimeOccupancy;
	};

	//�������ݼ���
	class Lane
	{
	public:

		/**
		* @brief: ���캯��
		* @param: id �������
		* @param: region ��������
		*/
		Lane(const std::string& id,Polygon region);

		/**
		* @brief: ��ȡ�������
		* @return: �������
		*/
		const std::string& Id() const;

		/**
		* @brief: ���ͻ������������
		* @param: detectRegion �������
		* @param: timeStamp ʱ���
		* @param: data ��������
		*/
		void PushVehicle(const Rectangle& detectRegion, long long timeStamp, const std::string& data);

		/**
		* @brief: ���ͷǻ������������
		* @param: detectRegion �������
		* @param: timeStamp ʱ���
		* @param: data ��������
		*/
		void PushBike(const Rectangle& detectRegion, long long timeStamp, const std::string& data);

		/**
		* @brief: �������˼������
		* @param: detectRegion �������
		* @param: timeStamp ʱ���
		* @param: data ��������
		*/
		void PushPedestrain(const Rectangle& detectRegion,long long timeStamp,const std::string& data);

		/**
		* @brief: �ռ�������������
		* @return: ������������
		*/
		LaneItem Collect();

	private:

		//�������
		std::string _id;

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

