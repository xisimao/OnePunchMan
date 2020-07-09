#pragma once
#include <string>

#include "StringEx.h"
#include "Sqlite.h"

namespace OnePunchMan
{
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

	//����
	class TrafficLane
	{
	public:
		//ͨ�����
		int ChannelIndex;
		//�������
		int LaneIndex;
		//��������
		std::string LaneName;
		//����
		std::string Region;
	};

	//��Ƶͨ��
	class TrafficChannel
	{
	public:
		TrafficChannel()
			:ChannelIndex(0), ChannelName(), ChannelUrl(), ChannelType(0), ChannelStatus(0)
			, Loop(true), OutputImage(false), OutputReport(false), OutputRecogn(false), GlobalDetect(false)
		{

		}
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
		//�Ƿ�ѭ��
		bool Loop;
		//�Ƿ����ͼƬ
		bool OutputImage;
		//�Ƿ������ⱨ��
		bool OutputReport;
		//�Ƿ����ʶ������
		bool OutputRecogn;
		//�Ƿ�ȫ�ּ��
		bool GlobalDetect;

		/**
		* @brief: ��ȡͨ��rtmp��ַ
		* @return: ͨ��rtmp��ַ
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://", ip, ":1935/live/", ChannelIndex);
		}

		/**
		* @brief: ��ȡͨ��http-flv��ַ
		* @return: ͨ��http-flv��ַ
		*/
		std::string FlvUrl(const std::string ip) const
		{
			return StringEx::Combine("http://",ip,":1936/live?port=1935&app=live&stream=", ChannelIndex);
		}
	};

	//���ݿ�
	class TrafficData
	{
	public:
		/**
		* @brief: ���캯��
		*/
		TrafficData();

		/**
		* @brief: ��������
		*/
		virtual ~TrafficData()= default;

		/**
		* @brief: ��ʼ�����ݿ�����
		* @param: dbName ���ݿ�����
		*/
		static void Init(const std::string& dbName);

		/**
		* @brief: ��ȡ���һ��������Ϣ
		* @return: ���һ��������Ϣ
		*/
		std::string LastError();

		/**
		* @brief: ��ȡ����ֵ
		* @param: key ������
		* @return: ����ֵ
		*/
		std::string GetParameter(const std::string& key);

		/**
		* @brief: ��ȡ����ֵ
		* @param: key ������
		* @param: value ����ֵ
		* @return: ���ý��
		*/
		bool SetParameter(const std::string& key, const std::string& value);
		
		/**
		* @brief: �������ݿ�
		*/
		virtual void UpdateDb();

	protected:
		//���ݿ�����
		static std::string _dbName;

		//����д��
		SqliteWriter _sqlite;
	};
}



