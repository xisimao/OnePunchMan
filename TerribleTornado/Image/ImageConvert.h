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
		* @param: width ͼƬ����
		* @param: height ͼƬ�߶�
		* @param: bgrBuffer д���bgr�ֽ���
		*/
		static void IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer);

		/**
		* @brief: bgrתjpg
		* @param: bgrBuffer bgr�ֽ���
		* @param: width ͼƬ����
		* @param: height ͼƬ�߶�
		* @param: jpgBuffer ����д���jpg�ֽ���
		* @param: jpgSize ����д���jpg�ֽ�������
		* @return: jpg�ֽ����ĳ���
		*/
		static int BgrToJpg(const unsigned char* bgrBuffer, int width, int height, unsigned char* jpgBuffer, int jpgSize);

		/**
		* @brief: nv21תyuv420sp
		* @param: nv21Buffer nv21Buffer�ֽ���
		* @param: width ͼƬ����
		* @param: height ͼƬ�߶�
		* @param: yuv420pBuffer ����д���yuv420p�ֽ���
		*/
		static void NV21ToYuv420p(const unsigned char* nv21Buffer, int width, int height, unsigned char* yuv420pBuffer);

		/**
		* @brief: iveתjpg base64
		* @param: bgrBuffer bgr�ֽ���
		* @param: width ͼƬ����
		* @param: height ͼƬ�߶�
		* @param: jpgBase64 ����д��base64�ַ���
		* @param: jpgBuffer ����д���jpg�ֽ���
		* @param: jpgSize ����д���jpg�ֽ�������
		* @return: jpg�ֽ����ĳ���
		*/
		static int BgrToJpgBase64(const unsigned char* bgrBuffer, int width, int height, std::string* jpgBase64, unsigned char* jpgBuffer, int jpgSize);

		/**
		* @brief: iveתjpg base64
		* @param: iveBuffer ive�ֽ���
		* @param: width ͼƬ����
		* @param: height ͼƬ�߶�
		* @param: bgrBuffer ת��ʱ�õ���bgr�ֽ���
		* @param: jpgBase64 ����д��base64�ַ���
		* @param: jpgBuffer ����д���jpg�ֽ���
		* @param: jpgSize ����д���jpg�ֽ�������
		* @return: jpg�ֽ����ĳ���
		*/
		static int IveToJpgBase64(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer, std::string* jpgBase64, unsigned char* jpgBuffer, int jpgSize);
		
		/**
		* @brief: mp4�ļ� base64
		* @param: filePath MP4�ļ�·��
		* @param: videoBuffer ������ʱ���MP4�ļ��ֽ���
		* @param: videoSize ������ʱ���MP4�ļ��ֽ����ĳ���
		* @param: base64 ����д��base64�ַ���
		*/
		static void Mp4ToBase64(const std::string& filePath,unsigned char* videoBuffer,int videoSize, std::string* base64);

		/**
		* @brief: jpgд��file
		* @param: jpgBuffer jpg�ֽ���
		* @param: jpgSize jpg�ֽ�������
		* @param: channelIndex ͨ�����
		* @param: frameIndex ֡���
		*/
		static void JpgToFile(unsigned char* jpgBuffer, int jpgSize, int channelIndex, unsigned int frameIndex);

		/**
		* @brief: ���Ƽ���
		* @param: image ���Ƶ�ͼƬ
		* @param: point ��
		* @param: scalar ��ɫ
		*/
		static void DrawPoint(cv::Mat* image, const Point& point, const cv::Scalar& scalar);

		/**
		* @brief: ���ƾ�������
		* @param: image ���Ƶ�ͼƬ
		* @param: rectangle ����
		* @param: scalar ��ɫ
		*/
		static void DrawRectangle(cv::Mat* image, const Rectangle& rectangle, const cv::Scalar& scalar);

		/**
		* @brief: ���ƶ��������
		* @param: image ���Ƶ�ͼƬ
		* @param: polygon �����
		* @param: scalar ��ɫ
		*/
		static void DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar);

		/**
		* @brief: ���Ƽ���
		* @param: image ���Ƶ�ͼƬ
		* @param: text ���Ƶ��ı�
		* @param: point �ı����ڵĻ��Ƶ�
		* @param: scalar ��ɫ
		*/
		static void DrawText(cv::Mat* image, const std::string& text,const Point& point, const cv::Scalar& scalar);
	};

}
