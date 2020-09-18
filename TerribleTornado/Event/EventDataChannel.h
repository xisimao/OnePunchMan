#pragma once
#include <queue>

#include "Thread.h"
#include "Sqlite.h"
#include "EventData.h"
#include "MqttChannel.h"

namespace OnePunchMan
{
	//�¼�����
	class EventData
	{
	public:
		EventData()
			:Id(0), ChannelIndex(), ChannelUrl(), LaneIndex(0), TimeStamp(0), Type(0)
		{

		}

		int Id;
		std::string Guid;
		int ChannelIndex;
		std::string ChannelUrl;
		int LaneIndex;
		long long TimeStamp;
		int Type;

		std::string GetTempImage(int imageIndex)
		{
			return StringEx::Combine(TrafficDirectory::TempDir, Guid, "_", imageIndex, ".jpg");
		}

		std::string GetImage(int imageIndex)
		{
			return StringEx::Combine(TrafficDirectory::FileDir, Guid, "_", imageIndex, ".jpg");
		}

		std::string GetTempVideo()
		{
			return StringEx::Combine(TrafficDirectory::TempDir, Guid, ".mp4");
		}

		std::string GetVideo()
		{
			return StringEx::Combine(TrafficDirectory::FileDir, Guid,".mp4");
		}

		std::string GetImageLink(int imageIndex)
		{
			return GetImageLink(Guid, imageIndex);
		}

		std::string GetVideoLink()
		{
			return GetVideoLink(Guid);
		}

		static std::string GetImageLink(const std::string& guid,int imageIndex)
		{
			return StringEx::Combine(TrafficDirectory::FileLink,"/", guid, "_", imageIndex, ".jpg");
		}

		static std::string GetVideoLink(const std::string& guid)
		{
			return StringEx::Combine(TrafficDirectory::FileLink, "/", guid, ".mp4");
		}
	};

	//�¼����ݲ����߳�
	class EventDataChannel:public ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: mqtt mqtt
		*/
		EventDataChannel(MqttChannel* mqtt);

		/**
		* @brief: ���캯��
		* @param: channelIndex ��Ƶ���
		* @return: �¼����ݼ���
		*/
		static std::vector<EventData> GetDatas(int channelIndex=0);

		/**
		* @brief: ����µ��¼�����
		* @param: width ͼƬ���
		* @param: height ͼƬ�߶�
		* @param: mqtt mqtt
		* @param: encodeChannel �����߳�
		* @param: dataChannel �����߳�
		*/
		void AddData(const EventData& data);

	protected:
		void StartCore();

	private:
		//��ౣ����¼���������
		const static int MaxDataCount;
		//IO mqtt����
		static const std::string EventTopic;
		//���ݿ�д��
		SqliteWriter _writer;
		//mqtt
		MqttChannel* _mqtt;
		//�¼�����ͬ����
		std::mutex _mutex;
		//��ǰ��������ݶ���
		std::queue<EventData> _datas;
		//��ǰҪ��ӵ����ݶ���
		std::queue<EventData> _tempDatas;
	};

}

