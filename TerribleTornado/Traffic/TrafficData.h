#pragma once
#include <string>

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
		std::string LaneId;
		//��������
		std::string LaneName;
		//�������
		int LaneIndex;
		//��������
		int Direction;
		//�����
		std::string DetectLine;
		//ֹͣ��
		std::string StopLine;
		//������1
		std::string LaneLine1;
		//������2
		std::string LaneLine2;
		//����
		std::string Region;
	};

	//��Ƶͨ��
	class TrafficChannel
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

		/**
		* @brief: ��ȡͨ��rtmp��ַ
		* @return: ͨ��rtmp��ַ
		*/
		std::string RtmpUrl(const std::string ip) const
		{
			return StringEx::Combine("rtmp://", ip, ":1935/live/", ChannelIndex);
		}
	};

}



