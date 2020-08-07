#pragma once
#include <string>

#include "StringEx.h"
#include "Sqlite.h"

namespace OnePunchMan
{
	//ͨ������
	enum class ChannelType
	{
		None=0,
		GB28181 = 1,
		RTSP = 2,
		File = 3
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

	//�������
	class GbParameter
	{
	public:
		GbParameter()
			:ServerIp(), ServerPort(0), SipPort(0)
			, SipType(0), GbId(), DomainId(), UserName(), Password()
		{

		}
		std::string ServerIp;
		int ServerPort;
		int SipPort;
		int SipType;
		std::string GbId;
		std::string DomainId;
		std::string UserName;
		std::string Password;
		
	};

	//�����豸
	class GbDevice
	{
	public:
		GbDevice()
			:DeviceId(0),DeviceName(),GbId(),DeviceIp(),DevicePort(0),UserName(),Password()
		{

		}
		int DeviceId;
		std::string DeviceName;
		std::string GbId;
		std::string DeviceIp;
		int DevicePort;
		std::string UserName;
		std::string Password;
	};

	//����ͨ��
	class GbChannel
	{
	public:
		GbChannel()
			:Id(0),ChannelId(),ChannelName(),DeviceId()
		{

		}
		int Id;
		std::string ChannelId;
		std::string ChannelName;
		std::string DeviceId;
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
		* @brief: ���ù������ֵ
		* @param: parameter �������
		* @return: ���ý��
		*/
		bool SetGbPrameter(const GbParameter& parameter);

		/**
		* @brief: ��ȡ�������
		* @return: �������
		*/
		GbParameter GetGbPrameter();

		/**
		* @brief: ��ȡ�����豸����
		* @return: �����豸����
		*/
		std::vector<GbDevice> GetGbDeviceList();

		/**
		* @brief: ��ӹ����豸
		* @param: device �����豸
		* @return: ��ӳɹ������������򷵻�-1
		*/
		int InsertGbDevice(const GbDevice& device);

		/**
		* @brief: ���¹����豸
		* @param: device �����豸
		* @return: ���½��
		*/
		bool UpdateGbDevice(const GbDevice& device);

		/**
		* @brief: ɾ�������豸
		* @param: deviceId �����豸���
		* @return: ɾ�����
		*/
		bool DeleteGbDevice(int deviceId);

		std::vector<GbChannel> GetGbChannelList(const std::string& gbId);

		/**
		* @brief: �������ݿ�
		*/
		virtual void UpdateDb();

	protected:
		/**
		* @brief: ���ͨ��
		* @param: sqlite ��ѯ���
		* @param: channel Ҫ����ͨ������
		*/
		void FillChannel(const SqliteReader& sqlite, TrafficChannel* channel);

		/**
		* @brief: ��ȡ��ѯ����sql���
		* @return: ��ѯ����sql���
		*/
		std::string GetChannelList();

		/**
		* @brief: ��ȡ��ѯ����sql���
		* @param: channelIndex ͨ�����
		* @return: ��ѯ����sql���
		*/
		std::string GetChannel(int channelIndex);

		/**
		* @brief: ��ȡ���ͨ��sql���
		* @param: channel Ҫ��ӵ�ͨ������
		* @return: ���ͨ��sql���
		*/
		std::string InsertChannel(const TrafficChannel* channel);

		/**
		* @brief: ��ȡɾ��ͨ��sql���
		* @param: channelIndex ͨ�����
		* @return: ɾ��ͨ��sql���
		*/
		std::string DeleteChannel(int channelIndex);

		/**
		* @brief: ��ȡ���ͨ��sql���
		* @return: ���ͨ��sql���
		*/
		std::string ClearChannel();

		//���ݿ�����
		static std::string _dbName;

		//����д��
		SqliteWriter _sqlite;
	};


}



