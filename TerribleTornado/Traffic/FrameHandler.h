#pragma once
#include "LogPool.h"
#include "IVEHandler.h"
#include "TrafficData.h"

#ifndef _WIN32
#include "mpi_vpss.h"
#include "mpi_ive.h"
#include "mpi_sys.h"
#endif // !_WIN32

namespace OnePunchMan
{
	class FrameHandler
	{
	public:
		/**
		* ���캯��
		* @param channelIndex ͨ�����
		* @param width �������Ƶ���
		* @param height �������Ƶ�߶�
		*/
		FrameHandler(int channelIndex,int width, int height);

		virtual ~FrameHandler();

		/**
		* ����һ֡��Ƶд�뵽bmp
		*/
		void WriteBmp();

#ifndef _WIN32
		/**
		* ������ʵ�ֵĴ���������
		* @param items ����������
		* @param timeStamp ʱ���
		* @param streamId ��Ƶ�����
		* @param taskId ������
		* @param iveBuffer ͼƬ�ֽ���
		* @param frameIndex ֡���
		* @param frameSpan ֡���ʱ��(����)
		* @return �������true��ʾ��Ҫ�������ͷ�֡�������ʾʵ�������ͷ�֡
		*/
		virtual bool HandleFrame(VIDEO_FRAME_INFO_S* frame);

	protected:
		/**
		* yuvתive
		* @return ת���ɹ�����true,���򷵻�false
		*/
		bool YuvToIve(VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

		//ͨ�����
		int _channelIndex;

		//�Ƿ���Ҫд����һ֡��bmp
		bool _writeBmp;

		//ͼ��ת��
		//yuv420sp
		int _yuvSize;
		//yuv����ʱ�ֽ���
		unsigned long long _yuv_phy_addr;
		unsigned char* _yuvBuffer;
		//ive����ʱ�ֽ���
		int _iveSize;
		unsigned long long _ive_phy_addr;
		unsigned char* _iveBuffer;

	private:
		//�������Ƶ���
		int _width;
		//�������Ƶ�߶�
		int _height;
		//iveͼ�񱣴�
		IVEHandler _iveHandler;

	};
}


