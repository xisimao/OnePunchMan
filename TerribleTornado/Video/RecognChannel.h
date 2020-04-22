#pragma once
#include <queue>
#include <mutex>

#include "SeemmoSDK.h"
#include "Thread.h"
#include "ChannelDetector.h"

namespace TerribleTornado
{
	//ʶ���߳�
	class RecognChannel :public Saitama::ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: recognIndex ����߳����
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: detectors ͨ����⼯��
		*/
		RecognChannel(int recognIndex,int width, int height, const std::vector<ChannelDetector*>& detectors);
	
		/**
		* @brief: ��������
		*/
		~RecognChannel();

		/**
		* @brief: ����guid����
		* @param: channelIndex ͨ�����
		* @param: guids guid����
		*/
		void PushGuids(int channelIndex,const std::vector<std::string>& guids);
		
		/**
		* @brief: ��ȡ�Ƿ��ʼ�����
		* @return: ��ʼ����ɷ���ture�����򷵻�false
		*/
		bool Inited();

		//��ʾһ��ʶ���߳̿��Խ��յ�ͨ������
		static const int ItemCount;

	protected:
		void StartCore();

	private:
		//guid������
		class GuidItem
		{
		public:
			//ͨ�����
			int ChannelIndex;
			//guid
			std::string Guid;
		};

		//��󻺴�����
		static const int MaxCacheCount;
		//�߳�����ʱ��(ms)
		static const int SleepTime;

		//�Ƿ��ʼ�����
		int _inited;
		//����߳����
		int _recognIndex;
		//ͨ����⼯��
		std::vector<ChannelDetector*> _detectors;
		//guid�������ͬ����
		std::mutex _mutex;
		//guid�������
		std::queue<GuidItem> _guidsCache;

		//recogn
		std::vector<uint8_t*> _bgrs;
		std::vector<const char*> _guids;
		std::string _param;
		std::vector<char> _result;

	};
}