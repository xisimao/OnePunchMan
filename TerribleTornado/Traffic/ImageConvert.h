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
		* ��ȡyuv�ֽ�������
		* @return yuv�ֽ�������
		*/
		int GetYuvSize();

		/**
		* ��ȡive�ֽ���
		* @return ive�ֽ���
		*/
		unsigned char* GetIveBuffer();

		/**
		* ��ȡbgr�ֽ���
		* @return bgr�ֽ���
		*/
		unsigned char* GetBgrBuffer();

		/**
		* ����yuv420sp(nv21)�ֽ������ݣ�ֻ��ת��1920*1080��
		* @param yuvBuffer nv21�ֽ���
		*/
		void YuvToIve(const unsigned char* yuvBuffer);

		/**
		* hisi ive_8uc3תbgr
		* @param iveBuffer ive�ֽ���
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		*/
		void IveToBgr(const unsigned char* iveBuffer,int width,int height);

		/**
		* bgrתjpg�ֽ���
		* @param brgBuffer bgr�ֽ���
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @return jpg�ֽ����ĳ���
		*/
		int BgrToJpg(const unsigned char* bgrBuffer, int width, int height);

		/**
		* bgrתbmp�ļ�
		* @param brgBuffer bgr�ֽ���
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param filePath д���ļ���·��
		*/
		void BgrToBmp(const unsigned char* bgrBuffer, int width, int height,const std::string& filePath);

		/**
		* jpgתbase64
		* @param jpgBuffer jpg�ֽ���
		* @param jpgSize jpg�ֽ�������
		* @return jpg base64�ַ���
		*/
		std::string JpgToBase64(const unsigned char* jpgBuffer, int jpgSize);

		/**
		* �ֽ���ת�ļ�
		* @param buffer �ֽ���
		* @param size �ֽ�������
		* @param filePath д���ļ���·��
		*/
		void BufferToFile(const unsigned char* buffer, int size, const std::string& filePath);

		/**
		* iveתjpg�ļ�
		* @param iveBuffer ive�ֽ���
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param filePath д���ļ�·��
		*/
		void IveToJpgFile(const unsigned char* iveBuffer, int width, int height, const std::string& filePath);

		/**
		* iveתjpg�ļ�
		* @param iveBuffer ive�ֽ���
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @return jpg base64�ַ���
		*/
		std::string IveToJpgBase64(const unsigned char* iveBuffer, int width, int height);

		/**
		* bgrתjpg�ļ�
		* @param bgrBuffer bgr�ֽ���
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param filePath д���ļ�·��
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

		//bgr�ֽ���
		unsigned char* _bgrBuffer;

		//jpg�ֽ�������
		int _jpgSize;
		//jpg�ֽ���
		unsigned char* _jpgBuffer;
	
	};



}

