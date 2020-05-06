#pragma once
#include <string>

#include "Shape.h"

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

	//���Ԫ��״̬
	enum class DetectStatus
	{
		//�״ν���
		New,
		//��������
		In,
		//��������
		Out
	};

	//���Ԫ������
	enum class DetectType
	{
		None = 0,
		Pedestrain = 1,
		Bike = 2,
		Motobike = 3,
		Car = 4,
		Tricycle = 5,
		Bus = 6,
		Van = 7,
		Truck = 8
	};

	//�����
	class DetectItem
	{
	public:
		//����
		//�������
		Rectangle Region;
		//���Ԫ������
		DetectType Type;

		//���
		//���Ԫ��״̬
		DetectStatus Status;
		//�ƶ�����
		double Distance;
	};

	//ʶ����
	class RecognItem
	{
	public:
		//ͨ�����
		int ChannelIndex;
		//guid
		std::string Guid;
		//���������
		int Type;
		//�������
		Rectangle Region;
		//���
		int Width;
		//�߶�
		int Height;
	};
}



