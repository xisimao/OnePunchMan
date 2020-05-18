#pragma once
#include "turbojpeg.h"
#include "opencv2/opencv.hpp"

#include "Shape.h"
#include "MqttChannel.h"
#include "JPGHandler.h"

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
		virtual void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp, std::string* param, const unsigned char* iveBuffer, int frameIndex,int frameSpan) = 0;

		/**
		* @brief: ����ʶ������
		* @param: item ʶ��������
		* @param: iveBuffer ͼƬ�ֽ���
		* @param: recognJson ʶ��json����
		*/
		virtual void HandleRecognize(const RecognItem& item, const unsigned char* iveBuffer, const std::string& recognJson)=0;

		/**
		* @brief: hisi ive_8uc3תbgr
		* @param: iveBuffer ive�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: bgrBuffer д���bgr�ֽ���
		*/
		static void IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer);
		
		/**
		* @brief: bgrתjpg
		* @param: bgrBuffer bgr�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: jpgBuffer д���jpg�ֽ���
		* @return: jpg�ֽ����ĳ���
		*/
		static int BgrToJpg(const unsigned char* bgrBuffer, int width, int height, unsigned char** jpgBuffer);

		/**
		* @brief: hisi ive_8uc3תbgr
		* @param: base64 ���ڴ��base64�ַ���
		* @param: jpgBuffer jpg�ֽ���
		* @param: jpgSize jpg�ֽ�������
		*/
		static void JpgToBase64(std::string* base64,const unsigned char* jpgBuffer, int jpgSize);

		/**
		* @brief: ���Ƽ������
		* @param: image ���Ƶ�ͼƬ
		* @param: polygon �����
		*/
		static void DrawPolygon(cv::Mat* image,const Polygon& polygon, const cv::Scalar& scalar);

		/**
		* @brief: ���Ƽ���
		* @param: image ���Ƶ�ͼƬ
		* @param: point ��
		*/
		static void DrawPoint(cv::Mat* image, const Point& point,const cv::Scalar& scalar);

	protected:
		//ͨ����Ϣ
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
		//bgr�ֽ���
		unsigned char* _bgrBuffer;
		//jpg�ֽ���
		unsigned char* _jpgBuffer;

		//�������
		//�Ƿ��ڵ���ģʽ
		bool _debug;
		//����ʱдjpg
		JPGHandler _jpgHandler;

	};

}

