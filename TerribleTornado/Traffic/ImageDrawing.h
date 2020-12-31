#pragma once
#include "opencv2/opencv.hpp"
#include "Shape.h"

namespace OnePunchMan
{
	class ImageDrawing
	{
	public:
		/**
		* 绘制检测点
		* @param image 绘制的图片
		* @param point 点
		* @param scalar 颜色
		*/
		static void DrawPoint(cv::Mat* image, const Point& point, const cv::Scalar& scalar);

		/**
		* 绘制矩形区域
		* @param image 绘制的图片
		* @param rectangle 矩形
		* @param scalar 颜色
		*/
		static void DrawRectangle(cv::Mat* image, const Rectangle& rectangle, const cv::Scalar& scalar);

		/**
		* 绘制多边形区域
		* @param image 绘制的图片
		* @param polygon 多边形
		* @param scalar 颜色
		*/
		static void DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar);

		/**
		* 绘制文本
		* @param image 绘制的图片
		* @param text 绘制的文本
		* @param point 文本基于的绘制点
		* @param scalar 颜色
		*/
		static void DrawString(cv::Mat* image, const std::string& text, const Point& point, const cv::Scalar& scalar);
	};
}


