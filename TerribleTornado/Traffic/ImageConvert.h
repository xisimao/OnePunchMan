#pragma once
#include "turbojpeg.h"
#include "opencv2/opencv.hpp"
#include "LogPool.h"
#include "Shape.h"

namespace OnePunchMan
{
	class ImageConvert
	{
	public:
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
		* @param: jpgBuffer ����д���jpg�ֽ���
		* @param: jpgSize ����д���jpg�ֽ�������
		* @return: jpg�ֽ����ĳ���
		*/
		static int BgrToJpg(const unsigned char* bgrBuffer, int width, int height, unsigned char* jpgBuffer, int jpgSize);

		/**
		* @brief: yuv420spתyuv420sp
		* @param: yuv420spBuffer yuv420sp�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: yuv420pBuffer ����д���yuv420p�ֽ���
		*/
		static void Yuv420spToYuv420p(const unsigned char* yuv420spBuffer, int width, int height, unsigned char* yuv420pBuffer);
		
		/**
		* @brief: iveתjpg base64
		* @param: iveBuffer ive�ֽ���
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: bgrBuffer ת��ʱ�õ���bgr�ֽ���
		* @param: jpgBase64 ����д��base64�ַ���
		* @param: jpgBuffer ����д���jpg�ֽ���
		* @param: jpgSize ����д���jpg�ֽ�������
		*/
		static void IveToJpgBase64(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer, std::string* jpgBase64, unsigned char* jpgBuffer, int jpgSize);
		
		///**
		//* @brief: ���Ƽ������
		//* @param: image ���Ƶ�ͼƬ
		//* @param: polygon �����
		//*/
		//static void DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar);

		///**
		//* @brief: ���Ƽ���
		//* @param: image ���Ƶ�ͼƬ
		//* @param: point ��
		//*/
		//static void DrawPoint(cv::Mat* image, const Point& point, const cv::Scalar& scalar);

	};

}

