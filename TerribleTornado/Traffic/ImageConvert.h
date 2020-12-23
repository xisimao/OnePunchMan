#pragma once
#include "turbojpeg.h"
#include "StringEx.h"
#include "LogPool.h"

#ifndef _WIN32
#include "hi_comm_video.h"
#include "mpi_ive.h"
#include "mpi_sys.h"
#endif // !_WIN32

namespace OnePunchMan
{
	class ImageConvert
	{
	
	public:
		ImageConvert(int width, int height,bool useYuvBuffer);

		~ImageConvert();

		/**
		* 获取yuv字节流长度
		* @return yuv字节流长度
		*/
		int GetYuvSize();

		/**
		* 获取ive字节流
		* @return ive字节流
		*/
		unsigned char* GetIveBuffer();

		/**
		* 获取bgr字节流
		* @return bgr字节流
		*/
		unsigned char* GetBgrBuffer();

		/**
		* 设置yuv420sp(nv21)字节流内容，只能转换1920*1080的
		* @param yuvBuffer nv21字节流
		*/
		void YuvToIve(const unsigned char* yuvBuffer);

		/**
		* hisi ive_8uc3转bgr
		* @param iveBuffer ive字节流
		* @param width 图片宽度
		* @param height 图片高度
		*/
		void IveToBgr(const unsigned char* iveBuffer,int width,int height);

		/**
		* bgr转jpg字节流
		* @param brgBuffer bgr字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @return jpg字节流的长度
		*/
		int BgrToJpg(const unsigned char* bgrBuffer, int width, int height);

		/**
		* bgr转bmp文件
		* @param brgBuffer bgr字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param filePath 写入文件的路径
		*/
		void BgrToBmp(const unsigned char* bgrBuffer, int width, int height,const std::string& filePath);

		/**
		* jpg转base64
		* @param jpgBuffer jpg字节流
		* @param jpgSize jpg字节流长度
		* @return jpg base64字符串
		*/
		std::string JpgToBase64(const unsigned char* jpgBuffer, int jpgSize);

		/**
		* 字节流转文件
		* @param buffer 字节流
		* @param size 字节流长度
		* @param filePath 写入文件的路径
		*/
		void BufferToFile(const unsigned char* buffer, int size, const std::string& filePath);

		/**
		* ive转jpg文件
		* @param iveBuffer ive字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param filePath 写入文件路径
		*/
		void IveToJpgFile(const unsigned char* iveBuffer, int width, int height, const std::string& filePath);

		/**
		* ive转jpg文件
		* @param iveBuffer ive字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @return jpg base64字符串
		*/
		std::string IveToJpgBase64(const unsigned char* iveBuffer, int width, int height);

		/**
		* bgr转jpg文件
		* @param bgrBuffer bgr字节流
		* @param width 图片宽度
		* @param height 图片高度
		* @param filePath 写入文件路径
		*/
		void BgrToJpgFile(const unsigned char* bgrBuffer, int width, int height, const std::string& filePath);

	private:
		int _width;
		int _height;

		//yuv420sp
		int _yuvSize;
		unsigned long long _yuv_phy_addr;
		unsigned char* _yuvBuffer;

		//ive
		int _iveSize;
		unsigned long long _ive_phy_addr;
		unsigned char* _iveBuffer;

		//bgr字节流
		unsigned char* _bgrBuffer;

		//jpg字节流长度
		int _jpgSize;
		//jpg字节流
		unsigned char* _jpgBuffer;
	
	};



}

