#pragma once
#include <string>

#include "Shape.h"

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
			:DetectItem(std::string(), 0, Rectangle())
		{

		}

		/**
		* @brief: ���캯��
		* @param: id ���Ԫ�ر��
		* @param: timeStamp ʱ���
		* @param: region ���Ԫ������
		*/
		DetectItem(const std::string& id, int timeStamp, const Rectangle& region)
			:Id(id), TimeStamp(timeStamp), Region(region)
		{

		}

		//���Ԫ�ر��
		std::string Id;
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
}