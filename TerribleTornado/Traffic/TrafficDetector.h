#pragma once
#include "Shape.h"
#include "MqttChannel.h"
#include "JPGHandler.h"

#include "turbojpeg.h"

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
			:Region(),Type(DetectType::None),Status(DetectStatus::Out)
		{

		}
		//����
		//�������
		Rectangle Region;
		//���Ԫ������
		DetectType Type;
		//���
		//���Ԫ��״̬
		DetectStatus Status;
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

	//�������
	enum class VideoStructType
	{
		Vehicle = 1,
		Bike = 2,
		Pedestrain = 3
	};

	class VideoStruct_Vehicle
	{
	public:
		VideoStruct_Vehicle()
			:CarType(0),CarColor(0),CarBrand(),PlateType(0),PlateNumber()
		{

		}

		int CarType;
		int CarColor;
		std::string CarBrand;
		int PlateType;
		std::string PlateNumber;
	};

	class VideoStruct_Bike
	{
	public:
		VideoStruct_Bike()
			:BikeType(0)
		{

		}
		int BikeType;
	};

	class VideoStruct_Pedestrain
	{
	public:
		VideoStruct_Pedestrain()
			:Sex(0),Age(0),UpperColor(0)
		{

		}

		int Sex;
		int Age;
		int UpperColor;
	};

	//ͨ�����
	class TrafficDetector
	{
	public:
		/**
		* @brief: ���캯��
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: mqtt mqtt
		* @param: debug �Ƿ��ڵ���ģʽ�����ڵ���ģʽ��������ߺ��bmp
		*/
		TrafficDetector(int width, int height, MqttChannel* mqtt, bool debug);

		/**
		* @brief: ��������
		*/
		virtual ~TrafficDetector();

		/**
		* @brief: ��ȡ�����Ƿ��ʼ���ɹ�
		* @return �����Ƿ��ʼ���ɹ�
		*/
		bool LanesInited() const;

		/**
		* @brief: ������ʵ�ֵĴ���������
		* @param: items ����������
		* @param: timeStamp ʱ���
		* @param: param ������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: frameIndex ֡���
		* @param: frameSpan ֡���ʱ��(����)
		*/
		virtual void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, const unsigned char* yuvBuffer, int frameIndex,int frameSpan) = 0;

		/**
		* @brief: ���������ʶ������
		* @param: recognItem ʶ��������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: vehicle ������ʶ������
		*/
		virtual void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle) {}
		
		/**
		* @brief: ����ǻ�����ʶ������
		* @param: recognItem ʶ��������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: bike �ǻ�����ʶ������
		*/
		virtual void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike) {}
		
		/**
		* @brief: ��������ʶ������
		* @param: recognItem ʶ��������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: pedestrain ����ʶ������
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
		//�����㷨ɸѡ�������
		std::string _param;
		//�Ƿ����ù���������
		bool _setParam;

		//bgr�ֽ�������
		int _bgrSize;
		//bgr�ֽ���
		unsigned char* _bgrBuffer;
		//jpg�ֽ�������
		int _jpgSize;
		//jpg�ֽ���
		unsigned char* _jpgBuffer;
	
		//�������
		//�Ƿ��ڵ���ģʽ
		bool _debug;
		//����ʱдjpg
		JPGHandler _jpgHandler;

	};

}

