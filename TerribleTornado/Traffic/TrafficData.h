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

	//ͨ��״̬
	enum class ChannelStatus
	{
		//����
		Normal = 1,
		//�޷�����ƵԴ
		InputError = 2,
		//��Ƶ�������
		OutputError = 3,
		//�������쳣
		DecoderError = 4,
		//�޷���ȡ��Ƶ����
		ReadError = 5,
		//�������
		DecodeError = 6,
		//׼��ѭ������
		ReadEOF_Restart = 7,
		//�ļ����Ž���
		ReadEOF_Stop = 8,
		//���ڳ�ʼ��
		Init = 9,
		//�����쳣(����)
		Disconnect = 10,
		//ͨ����ͬ��(����)
		NotFoundChannel = 11,
		//������ͬ��(����)
		NotFoundLane = 12,
		//����֡û�д���
		NotHandle = 13,
		//����avc1��ʽ����
		FilterError = 14
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
		TrafficLane()
			:ChannelIndex(),LaneIndex(),LaneName()
		{

		}
		//ͨ�����
		int ChannelIndex;
		//�������
		int LaneIndex;
		//��������
		std::string LaneName;
	};

	//��Ƶͨ��
	class TrafficChannel
	{
	public:
		TrafficChannel()
			:ChannelIndex(0), ChannelName(), ChannelUrl(), ChannelType(0), ChannelStatus(0)
			, Loop(true), OutputImage(false), OutputReport(false), OutputRecogn(false), GlobalDetect(false), DeviceId()
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
		//�����豸���
		std::string DeviceId;

		/**
		* ��ȡͨ��rtmp��ַ
		* @return ͨ��rtmp��ַ
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://", ip, ":1935/live/", ChannelIndex);
		}

		/**
		* ��ȡͨ��http-flv��ַ
		* @return ͨ��http-flv��ַ
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
			:Id(0), DeviceId(),DeviceName(),DeviceIp(),DevicePort(0),UserName(),Password()
		{

		}
		int Id;
		std::string DeviceId;
		std::string DeviceName;	
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
			:Id(0),ChannelId(),ChannelName()
		{

		}
		int Id;
		std::string ChannelId;
		std::string ChannelName;
	};

	//������Ϣ
	class TrafficDirectory
	{
	public:
		//�ļ���ʱ���Ŀ¼
		static std::string TempDir;
		//�ļ����Ŀ¼
		static std::string FileDir;
		//�ļ�Ŀ¼����
		static std::string FileLink;
		//ҳ��Ŀ¼
		static std::string WebDir;

		/**
		* ��ʼ��Ŀ¼����
		* @param jd json����
		*/
		static void Init(const std::string& webDir)
		{
			WebDir = webDir;
		}
	};

	//���ݿ�
	class TrafficData
	{
	public:
		/**
		* ���캯��
		*/
		TrafficData();

		/**
		* ��������
		*/
		virtual ~TrafficData()= default;

		/**
		* ��ʼ�����ݿ�����
		* @param dbName ���ݿ�����
		*/
		static void Init(const std::string& dbName);

		/**
		* ��ȡ���һ��������Ϣ
		* @return ���һ��������Ϣ
		*/
		std::string LastError();

		/**
		* ��ȡ����ֵ
		* @param key ������
		* @return ����ֵ
		*/
		std::string GetParameter(const std::string& key);

		/**
		* ��ȡ����ֵ
		* @param key ������
		* @param value ����ֵ
		* @return ���ý��
		*/
		bool SetParameter(const std::string& key, const std::string& value);
		
		/**
		* ���ù������ֵ
		* @param parameter �������
		* @return ���ý��
		*/
		bool SetGbPrameter(const GbParameter& parameter);

		/**
		* ��ȡ�������
		* @return �������
		*/
		GbParameter GetGbPrameter();

		/**
		* ��ȡ�����豸����
		* @return �����豸����
		*/
		std::vector<GbDevice> GetGbDeviceList();

		/**
		* ��ӹ����豸
		* @param device �����豸
		* @return ��ӳɹ������������򷵻�-1
		*/
		int InsertGbDevice(const GbDevice& device);

		/**
		* ���¹����豸
		* @param device �����豸
		* @return ���½��
		*/
		bool UpdateGbDevice(const GbDevice& device);

		/**
		* ɾ�������豸
		* @param deviceId �����豸���
		* @return ɾ�����
		*/
		bool DeleteGbDevice(int deviceId);

		/**
		* ��ѯ����ͨ������
		* @param deviceId �����豸���
		* @return ��ѯ���
		*/
		std::vector<GbChannel> GetGbChannelList(const std::string& deviceId);

		/**
		* �������ݿ�
		*/
		virtual void UpdateDb();

		//���ݿ�����
		static std::string DbName;

	protected:
		/**
		* ���ͨ��
		* @param sqlite ��ѯ���
		* @param channel Ҫ����ͨ������
		*/
		void FillChannel(const SqliteReader& sqlite, TrafficChannel* channel);

		/**
		* ��ȡ��ѯ����sql���
		* @return ��ѯ����sql���
		*/
		std::string GetChannelsSql();

		/**
		* ��ȡ��ѯ����sql���
		* @param channelIndex ͨ�����
		* @return ��ѯ����sql���
		*/
		std::string GetChannelSql(int channelIndex);

		/**
		* ��ȡ���ͨ��sql���
		* @param channel Ҫ��ӵ�ͨ������
		* @return ���ͨ��sql���
		*/
		std::string InsertChannelSql(const TrafficChannel* channel);

		/**
		* ��ȡɾ��ͨ��sql���
		* @param channelIndex ͨ�����
		* @return ɾ��ͨ��sql���
		*/
		std::string DeleteChannelSql(int channelIndex);

		/**
		* ��ȡ���ͨ��sql���
		* @return ���ͨ��sql���
		*/
		std::string ClearChannelSql();

		//����д��
		SqliteWriter _sqlite;
	};


}



