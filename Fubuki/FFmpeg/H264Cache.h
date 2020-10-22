#pragma once
#include <string>

#include "LogPool.h"
#include "FFmpegOutput.h"

namespace OnePunchMan
{
	//��Ƶ���
	class H264Cache
	{
	public:
		/**
		* ���캯��
		* @param channelIndex ͨ�����
		*/
		H264Cache(int channelIndex);

		/**
		* ��������
		*/
		~H264Cache();

		/**
		* ����h264��Ƶ��
		* @param data ��Ƶ���ֽ���
		* @param size ��Ƶ���ֽ�������
		*/
		void PushPacket(unsigned char* data, int size);

		/**
		* �����Ƶ���
		* @param outputUrl ��Ƶ�����ַ
		* @param iFrameCount ���I֡����
		*/
		bool AddOutputUrl(const std::string& outputUrl, int iFrameCount);

		/**
		* �Ƴ���Ƶ���
		* @param outputUrl ��Ƶ�����ַ
		*/
		void RemoveOutputUrl(const std::string& outputUrl);

		/**
		* �����Ƶ����Ƿ����
		* @param outputUrl ��Ƶ�����ַ
		* @return �����Ƶ����Ѿ���ɷ���true���Զ�ж��,���ǲ���ɾ���ļ�
		*/
		bool OutputFinished(const std::string& outputUrl);

		//I֡���
		const static int Gop;

		//��������������
		const static int MaxOutputCount;

	private:

		class FrameItem
		{
		public:
			unsigned char* Data;
			unsigned int Size;
		};
		/**
		* ��h264��Ƶ��д�뻺��
		* @param data ��Ƶ���ֽ���
		* @param size ��Ƶ���ֽ�������
		* @param frameIndex ��Ƶ����Ӧ��֡���
		* @param frameType ��Ƶ����Ӧ��֡����
		* @return ��ʾ�Ƿ�д�뻺��ɹ�
		*/
		bool WriteCache(unsigned char* data, int size, int frameIndex, FrameType frameType);

		//sps+pps��󳤶�
		const static int MaxExtraDataSize;
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
		unsigned char* _extraData;
		//�Ƿ��Ѿ���ȡ��sps֡
		int _spsSize;
		//�Ƿ��Ѿ���ȡ��pps֡
		int _ppsSize;

		//֡����
		std::vector<FrameItem> _frameCache;

		//�������
		std::mutex _mutex;
		std::map<std::string, FFmpegOutput*> _outputItems;
	};
}


