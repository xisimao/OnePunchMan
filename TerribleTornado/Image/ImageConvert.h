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
		* hisi ive_8uc3转bgr
		* @param iveBuffer ive字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param bgrBuffer 写入的bgr字节流
		*/
		static void IveToBgr(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer);



		/**
		* bgr转jpg
		* @param bgrBuffer bgr字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param jpgBuffer 用于写入的jpg字节流
		* @param jpgSize 用于写入的jpg字节流长度
		* @return jpg字节流的长度
		*/
		static int BgrToJpg(const unsigned char* bgrBuffer, int width, int height, unsigned char* jpgBuffer, int jpgSize);

		/**
		* nv21转yuv420sp
		* @param nv21Buffer nv21Buffer字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param yuv420pBuffer 用于写入的yuv420p字节流
		*/
		static void NV21ToYuv420p(const unsigned char* nv21Buffer, int width, int height, unsigned char* yuv420pBuffer);

		/**
		* ive转jpg base64
		* @param bgrBuffer bgr字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param jpgBuffer 用于写入的jpg字节流
		* @param jpgSize 用于写入的jpg字节流长度
		* @param jpgBase64 用于写入base64字符串
		* @return jpg字节流的长度
		*/
		static int BgrToJpgBase64(const unsigned char* bgrBuffer, int width, int height, unsigned char* jpgBuffer, int jpgSize, std::string* jpgBase64);
		
		/**
		* bgr转jpg文件
		* @param bgrBuffer bgr字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param jpgSize 用于写入的jpg字节流长度
		* @param jpgBase64 用于写入base64字符串
		* @param filePath 写入文件的路径
		*/
		static void BgrToJpgFile(const unsigned char* bgrBuffer, int width, int height,  unsigned char* jpgBuffer, int jpgSize, const std::string& filePath);

		/**
		* ive转jpg base64
		* @param iveBuffer ive字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param bgrBuffer 转换时用到的bgr字节流
		* @param jpgBuffer 用于写入的jpg字节流
		* @param jpgSize 用于写入的jpg字节流长度
		* @param jpgBase64 用于写入base64字符串
		* @return jpg字节流的长度
		*/
		static int IveToJpgBase64(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer, unsigned char* jpgBuffer, int jpgSize, std::string* jpgBase64);
		
		/**
		* ive转jpg文件
		* @param iveBuffer ive字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param bgrBuffer 转换时用到的bgr字节流
		* @param jpgBuffer 用于写入的jpg字节流
		* @param jpgSize 用于写入的jpg字节流长度
		* @param filePath 写入文件路径
		*/
		static void IveToJpgFile(const unsigned char* iveBuffer, int width, int height, unsigned char* bgrBuffer, unsigned char* jpgBuffer, int jpgSize, const std::string& filePath);

		/**
		* mp4文件 base64
		* @param filePath MP4文件路径
		* @param videoBuffer 用于临时存放MP4文件字节流
		* @param videoSize 用于临时存放MP4文件字节流的长度
		* @param base64 用于写入base64字符串
		*/
		static void Mp4ToBase64(const std::string& filePath,unsigned char* videoBuffer,int videoSize, std::string* base64);

		/**
		* jpg写入file
		* @param jpgBuffer jpg字节流
		* @param jpgSize jpg字节流长度
		* @param channelIndex 通道序号
		* @param frameIndex 帧序号
		*/
		static void JpgToFile(unsigned char* jpgBuffer, int jpgSize, int channelIndex, unsigned int frameIndex);

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
		* 绘制检测点
		* @param image 绘制的图片
		* @param text 绘制的文本
		* @param point 文本基于的绘制点
		* @param scalar 颜色
		*/
		static void DrawText(cv::Mat* image, const std::string& text,const Point& point, const cv::Scalar& scalar);
	};

}

