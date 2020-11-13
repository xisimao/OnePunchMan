#pragma once
#include "turbojpeg.h"
#include "opencv2/opencv.hpp"

#include "Shape.h"
#include "MqttChannel.h"
#include "ImageConvert.h"
#include "TrafficData.h"

namespace OnePunchMan
{
	//���Ԫ��״̬
	enum class DetectStatus
	{
		//��������
		Out,
		//�״ν���
		New,
		//��������
		In
	};

	//���Ԫ������
	enum class DetectType
	{
		None = 0,
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
		DetectItem()
			:Region(),Type(DetectType::None),Status(DetectStatus::Out), Distance(0.0)
		{

		}
		//����
		//�������
		Rectangle Region;
		//���Ԫ������
		DetectType Type;
		/**
		* ��ȡ������
		* @return ������
		*/
		int GetLength() const
		{
			switch (Type)
			{
			case DetectType::Car:
			case DetectType::Van:
				return 5;
			case DetectType::Tricycle:
				return 4;
			case DetectType::Bus:
				return 9;
			case DetectType::Truck:
				return 13;
			default:
				return 0;
			}
		}
		//���
		//���Ԫ��״̬
		DetectStatus Status;
		//��������ֹͣ�߾���(px)
		double Distance;
	};

	//ʶ����
	class RecognItem
	{
	public:
		RecognItem()
			:ChannelIndex(0), FrameIndex(0), FrameSpan(0), TaskId(0),Guid(),Type(DetectType::None),Region(),Width(0),Height(0)
		{

		}
		//ͨ�����
		int ChannelIndex;
		//֡���
		int FrameIndex;
		//֡���ʱ��(����)
		unsigned char FrameSpan;
		//�����
		unsigned char TaskId;
		//guid
		std::string Guid;
		//���������
		DetectType Type;
		//�������
		Rectangle Region;
		//���
		int Width;
		//�߶�
		int Height;
	};

	//�������
	enum class VideoStructType
	{
		Vehicle = 1,
		Bike = 2,
		Pedestrain = 3
	};

	//�������ṹ������
	class VideoStruct_Vehicle
	{
	public:
		VideoStruct_Vehicle()
			:LaneId(),LaneName(),Direction(0), Minute(0), Second(0), CarType(0),CarColor(0),CarBrand(),PlateType(0),PlateNumber()
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Direction;
		int Minute;
		int Second;
		int CarType;
		int CarColor;
		std::string CarBrand;
		int PlateType;
		std::string PlateNumber;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson()
		{
			std::string vehicleJson;
			JsonSerialization::SerializeValue(&vehicleJson, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&vehicleJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&vehicleJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&vehicleJson, "direction", Direction);
			JsonSerialization::SerializeValue(&vehicleJson, "carType", CarType);
			JsonSerialization::SerializeValue(&vehicleJson, "carColor", CarColor);
			JsonSerialization::SerializeValue(&vehicleJson, "carBrand", CarBrand);
			JsonSerialization::SerializeValue(&vehicleJson, "plateType", PlateType);
			JsonSerialization::SerializeValue(&vehicleJson, "plateNumber", PlateNumber);
			return vehicleJson;
		}
	};

	//�ǻ������ṹ������
	class VideoStruct_Bike
	{
	public:
		VideoStruct_Bike()
			:LaneId(), LaneName(), Direction(0), Minute(0), Second(0), BikeType(0)
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Direction;
		int Minute;
		int Second;
		int BikeType;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson()
		{
			std::string bikeJson;
			JsonSerialization::SerializeValue(&bikeJson, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&bikeJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&bikeJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&bikeJson, "direction", Direction);
			JsonSerialization::SerializeValue(&bikeJson, "bikeType", BikeType);
			return bikeJson;
		}
	};

	//���˽ṹ������
	class VideoStruct_Pedestrain
	{
	public:
		VideoStruct_Pedestrain()
			:LaneId(), LaneName(), Direction(0), Minute(0), Second(0), Sex(0),Age(0),UpperColor(0)
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Direction;
		int Minute;
		int Second;
		int Sex;
		int Age;
		int UpperColor;

		/**
		* ��ȡ���ݵ�json��ʽ
		* @return ���ݵ�json��ʽ
		*/
		std::string ToJson()
		{
			std::string pedestrainJson;
			JsonSerialization::SerializeValue(&pedestrainJson, "time", StringEx::Combine(Minute, ":", Second));
			JsonSerialization::SerializeValue(&pedestrainJson, "laneId", LaneId);
			JsonSerialization::SerializeValue(&pedestrainJson, "laneName", LaneName);
			JsonSerialization::SerializeValue(&pedestrainJson, "direction", Direction);
			JsonSerialization::SerializeValue(&pedestrainJson, "sex", Sex);
			JsonSerialization::SerializeValue(&pedestrainJson, "age", Age);
			JsonSerialization::SerializeValue(&pedestrainJson, "upperColor", UpperColor);
			return pedestrainJson;
		}
	};

	//ͨ�����
	class TrafficDetector
	{
	public:
		/**
		* ���캯��
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param mqtt mqtt
		*/
		TrafficDetector(int width, int height, MqttChannel* mqtt);

		/**
		* ��������
		*/
		virtual ~TrafficDetector() {};

		/**
		* ��ȡ�����Ƿ��ʼ���ɹ�
		* @return �����Ƿ��ʼ���ɹ�
		*/
		bool LanesInited() const;

		/**
		* ������ʵ�ֵĴ���������
		* @param items ����������
		* @param timeStamp ʱ���
		* @param streamId ��Ƶ�����
		* @param taskId ������
		* @param iveBuffer ͼƬ�ֽ���
		* @param frameIndex ֡���
		* @param frameSpan ֡���ʱ��(����)
		*/
		virtual void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp,unsigned long long streamId, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex, unsigned char frameSpan) = 0;
		
		/**
		* �������
		* @param taskId ������
		*/
		virtual void FinishDetect(unsigned char taskId) {};

		/**
		* ���������ʶ������
		* @param recognItem ʶ��������
		* @param iveBuffer ͼƬ�ֽ���
		* @param vehicle ������ʶ������
		*/
		virtual void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle) {}
		
		/**
		* ����ǻ�����ʶ������
		* @param recognItem ʶ��������
		* @param iveBuffer ͼƬ�ֽ���
		* @param bike �ǻ�����ʶ������
		*/
		virtual void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike) {}
		
		/**
		* ��������ʶ������
		* @param recognItem ʶ��������
		* @param iveBuffer ͼƬ�ֽ���
		* @param pedestrain ����ʶ������
		*/
		virtual void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain) {}

	protected:
		//��Ƶ���
		int _channelIndex;
		//��Ƶ��ַ
		std::string _channelUrl;
		//��Ƶ���
		int _width;
		//��Ƶ�߶�
		int _height;
		//mqtt
		MqttChannel* _mqtt;

		//������ʼ���Ƿ�ɹ�
		bool _lanesInited;

		//bgr�ֽ�������
		int _bgrSize;
		//jpg�ֽ�������
		int _jpgSize;
	};

}

