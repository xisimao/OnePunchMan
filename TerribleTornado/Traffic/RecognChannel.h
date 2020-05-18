#pragma once
#include <queue>
#include <mutex>

#include "SeemmoSDK.h"
#include "Thread.h"
#include "TrafficDetector.h"
#include "IVE_8UC3Handler.h"

namespace OnePunchMan
{
	//ʶ���߳�
	class RecognChannel :public ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: recognIndex ����߳����
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: detectors ͨ����⼯��
		*/
		RecognChannel(int recognIndex,int width, int height, const std::vector<TrafficDetector*>& detectors);
	
		/**
		* @brief: ��������
		*/
		~RecognChannel();

		/**
		* @brief: ����ʶ���������
		* @param: items ʶ���������
		*/
		void PushItems(const std::vector<RecognItem> items);
		
		/**
		* @brief: ��ȡ�Ƿ��ʼ�����
		* @return: ��ʼ����ɷ���ture�����򷵻�false
		*/
		bool Inited();

		/**
		* @brief: ��ȡ��ǰ�����е���������
		* @return: ��ǰ�����е���������
		*/
		int Size();

		//��ʾһ��ʶ���߳̿��Խ��յ�ͨ������
		static const int ItemCount;

	protected:
		void StartCore();

	private:
		//��󻺴�����
		static const int MaxCacheCount;
		//�߳�����ʱ��(ms)
		static const int SleepTime;

		//�Ƿ��ʼ�����
		int _inited;
		//����߳����
		int _recognIndex;
		//ͨ����⼯��
		std::vector<TrafficDetector*> _detectors;
		//guid�������ͬ����
		std::timed_mutex _queueMutex;
		//guid�������
		std::queue<RecognItem> _items;

		//recogn
		std::vector<uint8_t*> _bgrs;
		std::vector<const char*> _guids;
		std::string _param;
		std::vector<char> _result;
	};
}