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
		* @brief: hisi ive_8uc3转bgr
		* @param: iveBuffer ive字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: bgrBuffer 写入的bgr字节流
		*/
		static void IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer);

		/**
		* @brief: bgr转jpg
		* @param: bgrBuffer bgr字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: jpgBuffer 用于写入的jpg字节流
		* @param: jpgSize 用于写入的jpg字节流长度
		* @return: jpg字节流的长度
		*/
		static int BgrToJpg(const unsigned char* bgrBuffer, int width, int height, unsigned char* jpgBuffer, int jpgSize);

		/**
		* @brief: nv21转yuv420sp
		* @param: nv21Buffer nv21Buffer字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: yuv420pBuffer 用于写入的yuv420p字节流
		*/
		static void NV21ToYuv420p(const unsigned char* nv21Buffer, int width, int height, unsigned char* yuv420pBuffer);

		/**
		* @brief: ive转jpg base64
		* @param: bgrBuffer bgr字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: jpgBase64 用于写入base64字符串
		* @param: jpgBuffer 用于写入的jpg字节流
		* @param: jpgSize 用于写入的jpg字节流长度
		* @return: jpg字节流的长度
		*/
		static int BgrToJpgBase64(const unsigned char* bgrBuffer, int width, int height, std::string* jpgBase64, unsigned char* jpgBuffer, int jpgSize);

		/**
		* @brief: ive转jpg base64
		* @param: iveBuffer ive字节流
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: bgrBuffer 转换时用到的bgr字节流
		* @param: jpgBase64 用于写入base64字符串
		* @param: jpgBuffer 用于写入的jpg字节流
		* @param: jpgSize 用于写入的jpg字节流长度
		* @return: jpg字节流的长度
		*/
		static int IveToJpgBase64(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer, std::string* jpgBase64, unsigned char* jpgBuffer, int jpgSize);
		
		/**
		* @brief: jpg写入file
		* @param: jpgBuffer jpg字节流
		* @param: jpgSize jpg字节流长度
		* @param: channelIndex 通道序号
		* @param: frameIndex 帧序号
		*/
		static void JpgToFile(unsigned char* jpgBuffer, int jpgSize, int channelIndex, unsigned int frameIndex);

		/**
		* @brief: 绘制检测区域
		* @param: image 绘制的图片
		* @param: polygon 多边形
		*/
		static void DrawPolygon(cv::Mat* image, const Polygon& polygon, const cv::Scalar& scalar);

		/**
		* @brief: 绘制检测点
		* @param: image 绘制的图片
		* @param: point 点
		*/
		static void DrawPoint(cv::Mat* image, const Point& point, const cv::Scalar& scalar);

	};

}

