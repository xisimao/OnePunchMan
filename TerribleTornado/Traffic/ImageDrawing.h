#pragma once
#include "opencv2/opencv.hpp"
#include "Shape.h"

namespace OnePunchMan
{
	class ImageDrawing
	{
	public:
		/**
		* ���Ƽ���
		* @param image ���Ƶ�ͼƬ
		* @param point ��
		* @param scalar ��ɫ
		*/
		static void DrawPoint(cv::Mat* image, const Point& point, const cv::Scalar& scalar);

		/**
		* ���ƾ�������
		* @param image ���Ƶ�ͼƬ
		* @param rectangle ����
		* @param scalar ��ɫ
		*/
		static void DrawRectangle(cv::Mat* image, const Rectangle& rectangle, const cv::Scalar& scalar);

		/**
		* ���ƶ��������
		* @param image ���Ƶ�ͼƬ
		* @param polygon �����
		* @param scalar ��ɫ
		*/
		static void DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar);

		/**
		* �����ı�
		* @param image ���Ƶ�ͼƬ
		* @param text ���Ƶ��ı�
		* @param point �ı����ڵĻ��Ƶ�
		* @param scalar ��ɫ
		*/
		static void DrawString(cv::Mat* image, const std::string& text, const Point& point, const cv::Scalar& scalar);
	};
}


