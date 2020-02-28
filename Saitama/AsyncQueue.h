//#pragma once
//#include <queue>
//#include <vector>
//
//#include "Thread.h"
//
//namespace Saitama
//{
//	//��Ƶͨ�������߳�
//	class MessageChannel :public ThreadObject
//	{
//	public:
//
//		/**
//		* @brief: ���캯��
//		*/
//		VideoChannel();
//
//		/**
//		* @brief: ��������
//		*/
//		~VideoChannel();
//
//		/**
//		* @brief: ���³�������
//		* @param: lanes ��������
//		*/
//		void UpdateLanes(const std::vector<Lane*>& lanes);
//
//		/**
//		* @brief: ��������
//		* @param: data ��Ƶͨ���������
//		*/
//		void Push(const std::string& data);
//
//		/**
//		* @brief: �ռ���Ƶͨ����������
//		* @return: ��Ƶͨ����������
//		*/
//		std::vector<LaneItem> Collect();
//
//		//����IO�ı��¼�
//		Observable<IOChangedEventArgs> IOChanged;
//
//		//ͨ�����
//		std::string Id;
//
//		//ͨ�����
//		int Index;
//
//	protected:
//
//		void StartCore();
//
//		/**
//		* @brief: ��json�����л�ȡ������
//		* @param: json json����
//		* @param: key ������ֶεļ�
//		* @return: ������
//		*/
//		std::vector<DetectItem> GetDetectItems(const std::string& json, const std::string& key, long long timeStamp);
//
//		/**
//		* @brief: ��ճ�������
//		* @return: �������
//		*/
//		void ClearLanes();
//
//	private:
//
//		//ͬ����
//		std::mutex _dataMutex;
//		//���ݶ���
//		std::queue<std::string> _datas;
//
//		//ͬ����
//		std::mutex _laneMutex;
//		//��������
//		std::vector<Lane*> _lanes;
//	};
//}
//
//
