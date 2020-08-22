#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegInput.h"
#include "Mp4Output.h"

namespace OnePunchMan
{
	//��Ƶ���
	class H264Cache
	{
	public:
		/**
		* @brief: ���캯��
		* @param: channelIndex ͨ�����
		*/
		H264Cache(int channelIndex);

		/**
		* @brief: ��������
		*/
		~H264Cache();

		/**
		* @brief: ����h264��Ƶ��
		* @param: data ��Ƶ���ֽ���
		* @param: size ��Ƶ���ֽ�������
		*/
		void PushPacket(unsigned char* data, int size);

		/**
		* @brief: �����Ƶ֡����
		*/
		void ClearCache();

		/**
		* @brief: �����Ƶ���
		* @param: outputUrl ��Ƶ�����ַ
		* @param: iFrameCount ���I֡����
		*/
		bool AddOutputUrl(const std::string& outputUrl, int iFrameCount);

		bool OutputFinished(const std::string& outputUrl);

		//I֡���
		const static int Gop;

	private:

		class FrameItem
		{
		public:
			unsigned char* Data;
			unsigned int Size;
		};
		/**
		* @brief: ��h264��Ƶ��д�뻺��
		* @param: data ��Ƶ���ֽ���
		* @param: size ��Ƶ���ֽ�������
		* @param: frameIndex ��Ƶ����Ӧ��֡���
		* @param: frameType ��Ƶ����Ӧ��֡����
		* @return: ��ʾ�Ƿ�д�뻺��ɹ�
		*/
		bool WriteCache(unsigned char* data, int size, int frameIndex, int frameType);

		//ͬʱ����������
		const static int MaxOutputCount;
		//֡����󳤶�
		const static int FrameSize;

		//ͨ�����
		int _channelIndex;

		//��ǰi֡�����
		int _iFrameIndex;
		//��ǰp֡�����
		int _pFrameIndex;
		//��ǰ����֡������
		unsigned int _frameCount;

		//�����Ƶ����
		AVCodecParameters _avParameters;
		//sps+pps
		unsigned char* _extradata;
		//sps���ݳ���
		int _spsSize;
		//pps���ݳ���
		int _ppsSize;

		//֡����
		std::vector<FrameItem> _frameCache;

		//�������
		std::mutex _mutex;
		std::map<std::string, Mp4Output*> _outputItems;

		Mp4Output _rtmpOutput;
	};
}


