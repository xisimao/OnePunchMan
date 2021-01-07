#pragma once
#include "Seemmo_SDK.h"
#include "Seemmo_DecodeChannel.h"
#include "Seemmo_RecognChannel.h"
#include "FlowDetector.h"
#include "EventDetector.h"

namespace OnePunchMan
{
	//����߳�
	class Seemmo_DetectChannel :public ThreadObject
	{
	public:
		/**
		* ���캯��
		* @param detectIndex ������
		* @param width ��Ƶ�������
		* @param height ��Ƶ�����߶�
		*/
		Seemmo_DetectChannel(int detectIndex,int width, int height);

		/**
		* ��ǰ����߳��Ƿ�δ��ʼ��
		* @return ���δ��ʼ�����ߵ�ǰ�߳����ڽ��м�ⷵ��true,���򷵻�false
		*/
		bool Inited();

		/**
		* �����Ҫ����ͨ�����
		* @param channelIndex ͨ�����
		*/
		void SetRecogn(Seemmo_RecognChannel* recogn);

		/**
		* �����Ҫ����ͨ�����
		* @param channelIndex ͨ�����
		* @param decode �����߳�
		* @param flowDetector �������
		* @param eventDetector �¼����
		*/
		void AddChannel(int channelIndex,Seemmo_DecodeChannel* decode, FlowDetector* flowDetector,EventDetector* eventDetector);

		/**
		* ����ͨ��
		* @param channel ͨ��
		*/
		void UpdateChannel(const TrafficChannel& channel);

		/**
		* ����һ֡��Ƶд�뵽bmp
		* * @param channelIndex ͨ�����
		*/
		void Screenshot(int channelIndex);

	protected:
		void StartCore();

	private:
		//��Ƶ֡����
		class ChannelItem
		{
		public:
			ChannelItem()
				: ChannelIndex(0), Param(), WriteBmp(false), Decode(NULL), Flow(NULL), Event(NULL), OutputDetect(false), OutputImage(false)
			{

			}
			//ͨ�����
			int ChannelIndex;
			//��������
			std::string Param;
			//�Ƿ���Ҫ��ȡbmpͼƬ
			bool WriteBmp;
			//����
			Seemmo_DecodeChannel* Decode;
			//�������
			FlowDetector* Flow;
			//�¼����
			EventDetector* Event;
			//�Ƿ�����������
			bool OutputDetect;
			//�Ƿ����ͼƬ
			bool OutputImage;
		};

		/**
		* ��json�����л�ȡ������
		* @param items ������
		* @param jd json����
		* @param key �������,������,�ǻ����ͺ�����
		*/
		void GetDetecItems(std::map<std::string, DetectItem>* items, const JsonDeserialization& jd, const std::string& key);

		/**
		* ��json�����л�ȡʶ�����
		* @param items ʶ�����
		* @param jd json����
		* @param key �������,������,�ǻ����ͺ�����
		* @param channelIndex ͨ�����
		* @param frameIndex ֡���
		* @param frameSpan ֡���ʱ��(����)
		* @param taskId �����
		*/
		void GetRecognItems(std::vector<RecognItem>* items, const JsonDeserialization& jd, const std::string& key,int channelIndex, int frameIndex,unsigned char frameSpan,unsigned char taskId);

		//��ѯ��˯��ʱ��(����)
		static const int SleepTime;

		//�߳��Ƿ��ʼ�����
		bool _inited;
		//������
		int _detectIndex;
		//ͼƬ���
		int _width;
		//ͼƬ�߶�
		int _height;

		//��Ƶ�������ͬ����
		std::mutex _channelMutex;
		//��Ƶ֡����
		std::map<int,ChannelItem> _channelItems;
		//����߳�
		Seemmo_RecognChannel* _recogn;
		//ͼ��ת��
		ImageConvert _image;

		//detect
		std::vector<uint8_t*> _ives;
		std::vector<int> _indexes;
		std::vector<uint64_t> _timeStamps;
		std::vector<uint32_t> _widths;
		std::vector<uint32_t> _heights;
		std::vector<const char*> _params;
		std::vector<char> _result;
	};
}

