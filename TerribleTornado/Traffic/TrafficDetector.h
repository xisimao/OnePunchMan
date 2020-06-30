#pragma once
#include "turbojpeg.h"
#include "opencv2/opencv.hpp"

#include "Shape.h"
#include "MqttChannel.h"
#include "IVEHandler.h"
#include "ImageConvert.h"

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
		RecognItem()
			:ChannelIndex(0), FrameIndex(0), FrameSpan(0), TaskId(0),Guid(),Type(0),Region(),Width(0),Height(0)
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
			:LaneId(),LaneName(), Minute(0), Second(0), CarType(0),CarColor(0),CarBrand(),PlateType(0),PlateNumber()
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Minute;
		int Second;
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
			:LaneId(), LaneName(), Minute(0), Second(0), BikeType(0)
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Minute;
		int Second;
		int BikeType;
	};

	class VideoStruct_Pedestrain
	{
	public:
		VideoStruct_Pedestrain()
			:LaneId(), LaneName(),Minute(0), Second(0), Sex(0),Age(0),UpperColor(0)
		{

		}
		std::string LaneId;
		std::string LaneName;
		int Minute;
		int Second;
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
		*/
		TrafficDetector(int width, int height, MqttChannel* mqtt);

		/**
		* @brief: ��������
		*/
		virtual ~TrafficDetector() {};

		/**
		* @brief: ��ȡ�����Ƿ��ʼ���ɹ�
		* @return �����Ƿ��ʼ���ɹ�
		*/
		bool LanesInited() const;

		/**
		* @brief: ����һ֡��Ƶд�뵽bmp
		*/
		void WriteBmp();

		/**
		* @brief: ��ȡĬ�ϼ�����
		* @return: Ĭ�ϼ�����
		*/
		std::string GetDetectParam();

		/**
		* @brief: ������ʵ�ֵĴ���������
		* @param: items ����������
		* @param: timeStamp ʱ���
		* @param: param ������
		* @param: taskId ������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: frameIndex ֡���
		* @param: frameSpan ֡���ʱ��(����)
		*/
		virtual void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex, unsigned char frameSpan) = 0;
		
		/**
		* @brief: �������
		* @param: taskId ������
		*/
		virtual void FinishDetect(unsigned char taskId) {};

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
		/**
		* @brief: ��ȡ����������
		* @param: regions �������򼯺ϣ�json�ַ���
		* @return: ����������
		*/
		std::string GetDetectParam(const std::string& regions);

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
		//�Ƿ���Ҫд����һ֡��bmp
		bool _writeBmp;

		//bgr�ֽ�������
		int _bgrSize;
		//jpg�ֽ�������
		int _jpgSize;

		//iveд��bmp
		IVEHandler _iveHandler;

	};

}

