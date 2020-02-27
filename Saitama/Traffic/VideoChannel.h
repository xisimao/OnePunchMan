#pragma once
#include <queue>
#include <vector>

#include "Lane.h"
#include "Thread.h"

namespace Saitama
{
	//��Ƶͨ�������߳�
	class VideoChannel:public ThreadObject
	{
	public:

		/**
		* @brief: ���캯��
		*/
		VideoChannel();

		/**
		* @brief: ��������
		*/
		~VideoChannel();

		/**
		* @brief: ���³�������
		* @param: lanes ��������
		*/
		void UpdateLanes(const std::vector<Lane*>& lanes);

		/**
		* @brief: ��������
		* @param: data ��Ƶͨ���������
		*/
		void Push(const std::string& data);

		/**
		* @brief: �ռ���Ƶͨ����������
		* @return: ��Ƶͨ����������
		*/
		std::string Collect();

	protected:

		void StartCore();

		/**
		* @brief: �Ӽ�������л�ȡ�������
		* @return: �������
		*/
		Rectangle GetRegion(const std::string& data);

		/**
		* @brief: ��ճ�������
		* @return: �������
		*/
		void ClearLanes();

	private:

		//ͬ����
		std::mutex _dataMutex;
		//���ݶ���
		std::queue<std::string> _datas;

		//ͬ����
		std::mutex _laneMutex;
		//��������
		std::vector<Lane*> _lanes;
	};
}


