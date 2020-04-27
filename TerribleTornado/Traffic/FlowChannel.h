#pragma once
#include <vector>
#include <string>

#include "Sqlite.h"
#include "StringEx.h"

namespace OnePunchMan
{
	//����
	class Lane
	{
	public:
		//ͨ�����
		int ChannelIndex;
		//�������
		std::string LaneId;
		//��������
		std::string LaneName;
		//�������
		int LaneIndex;
		//��������
		int LaneType;
		//��������
		int Direction;
		//��������
		int FlowDirection;
		//��������
		int Length;
		//io�������ַ
		std::string IOIp;
		//io������˿�
		int IOPort;
		//io����������
		int IOIndex;	
		//�����
		std::string DetectLine;
		//ֹͣ��
		std::string StopLine;
		//������1
		std::string LaneLine1;
		//������2
		std::string LaneLine2;
		std::string Region;
	};

	//ͨ������
	enum class ChannelType
	{
		GB28181 = 1,
		RTSP = 2,
		File = 3,
		ONVIF = 4
	};

	//�豸״̬
	enum class DeviceStatus
	{
		Normal = 1,
		Error = 2
	};

	//������Ƶͨ��
	class FlowChannel
	{
	public:
		//ͨ�����
		int ChannelIndex;
		//ͨ������
		std::string ChannelName;
		//ͨ����ַ
		std::string ChannelUrl;
		//ͨ������
		int ChannelType;
		//ͨ��״̬
		int ChannelStatus;
		//��������
		std::vector<Lane> Lanes;

		/**
		* @brief: ��ȡͨ��rtmp��ַ
		* @return: ͨ��rtmp��ַ
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://",ip,":1935/live/", ChannelIndex);
		}
	};

	//������Ƶͨ�����ݿ����
	class FlowChannelData
	{
	public:
		/**
		* @brief: ��ѯͨ���б�
		* @return: ͨ���б�
		*/
		std::vector<FlowChannel> GetList();

		/**
		* @brief: ��ѯ����ͨ��
		* @param: channelIndex ͨ�����
		* @return: ͨ��
		*/
		FlowChannel Get(int channelIndex);

		/**
		* @brief: ���ͨ��
		* @param: channel ͨ��
		* @return: ��ӽ��
		*/
		bool Insert(const FlowChannel& channel);

		/**
		* @brief: ����ͨ��
		* @param: channel ͨ��
		* @return: ���ý��
		*/
		bool Set(const FlowChannel& channel);

		/**
		* @brief: ����ͨ������
		* @param: channels ͨ������
		* @return: ���ý��
		*/
		bool SetList(const std::vector<FlowChannel>& channels);
		
		/**
		* @brief: ɾ��ͨ��
		* @param: channel ͨ��
		* @return: ɾ���ṹ
		*/
		bool Delete(int channelIndex);

		/**
		* @brief: ���ͨ��
		*/
		void Clear();

		/**
		* @brief: ��ȡ���һ��������Ϣ
		* @return: ���һ��������Ϣ
		*/
		std::string LastError();

		//ͨ������
		static const int ChannelCount;

	private:
		/**
		* @brief: ���ͨ��
		* @param: sqlite ��ѯ���
		* @return: ͨ��
		*/
		FlowChannel FillChannel(const SqliteReader& sqlite);

		//����д��
		SqliteWriter _sqlite;
	};
}


