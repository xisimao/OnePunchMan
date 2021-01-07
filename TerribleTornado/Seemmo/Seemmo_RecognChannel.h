#pragma once
#include <queue>
#include <mutex>

#include "Seemmo_SDK.h"
#include "Thread.h"
#include "FlowDetector.h"

namespace OnePunchMan
{
	//ʶ���߳�
	class Seemmo_RecognChannel :public ThreadObject
	{
	public:
		/**
		* ���캯��
		* @param recognIndex ����߳����
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param detectors ͨ����⼯��
		*/
		Seemmo_RecognChannel(int recognIndex,int width, int height, std::vector<FlowDetector*>* detectors);

		/**
		* ��������
		*/
		~Seemmo_RecognChannel();
		/**
		* ����ʶ���������
		* @param items ʶ���������
		*/
		void PushItems(const std::vector<RecognItem>& items);
		
		/**
		* ��ȡ�Ƿ��ʼ�����
		* @return ��ʼ����ɷ���ture,���򷵻�false
		*/
		bool Inited();

		/**
		* ��ȡ��ǰ�����е���������
		* @return ��ǰ�����е���������
		*/
		int Size();

	protected:
		void StartCore();

	private:
		//��󻺴�����
		static const int MaxCacheCount;
		//�߳�����ʱ��(ms)
		static const int SleepTime;
		//ʶ������
		static const std::string RecognTopic;

		//�Ƿ��ʼ�����
		int _inited;
		//����߳����
		int _recognIndex;
		//ͨ����⼯��
		std::vector<FlowDetector*>* _detectors;
		//guid�������ͬ����
		std::mutex _queueMutex;
		//guid�������
		std::queue<RecognItem> _items;

		//recogn
		std::vector<uint8_t*> _bgrs;
		std::vector<const char*> _guids;
		std::string _param;
		std::vector<char> _result;
	};
}