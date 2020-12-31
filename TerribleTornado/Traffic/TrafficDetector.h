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
			:Id(),Region(),Type(DetectType::None),Status(DetectStatus::Out), Distance(0.0)
		{

		}
		std::string Id;
		//����
		//�������
		Rectangle Region;
		//���Ԫ������
		DetectType Type;

		//���
		//���Ԫ��״̬
		DetectStatus Status;
		//��������ֹͣ�߾���(px)
		double Distance;

		std::string ToJson()
		{
			std::string json;
			JsonSerialization::SerializeValue(&json, "id", Id);
			JsonSerialization::SerializeValue(&json, "region", Region.ToJson());
			JsonSerialization::SerializeValue(&json, "type", static_cast<int>(Type));
			return json;
		}
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

