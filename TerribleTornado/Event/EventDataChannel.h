#pragma once
#include <queue>

#include "Thread.h"
#include "Sqlite.h"
#include "EventData.h"
#include "MqttChannel.h"

namespace OnePunchMan
{
	//事件数据
	class EventData
	{
	public:
		EventData()
			:Id(0), ChannelUrl(), LaneIndex(0), TimeStamp(0), Type(0)
		{

		}

		int Id;
		std::string Guid;
		std::string ChannelUrl;
		int LaneIndex;
		long long TimeStamp;
		int Type;

		std::string GetTempImage(int imageIndex)
		{
			return StringEx::Combine("../temp/", Guid, "_", imageIndex, ".jpg");
		}

		std::string GetImage(int imageIndex)
		{
			return StringEx::Combine("../images/", Guid, "_", imageIndex, ".jpg");
		}

		std::string GetTempVideo()
		{
			return StringEx::Combine("../temp/", Guid, ".mp4");
		}

		std::string GetVideo()
		{
			return StringEx::Combine("../images/", Guid,".mp4");
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
			return StringEx::Combine("images/", guid, "_", imageIndex, ".jpg");
		}

		static std::string GetVideoLink(const std::string& guid)
		{
			return StringEx::Combine("images/", guid, ".mp4");
		}
	};

	//事件数据操作线程
	class EventDataChannel:public ThreadObject
	{
	public:
		EventDataChannel(MqttChannel* mqtt);

		void Init();

		void Push(const EventData& data);

	protected:
		void StartCore();

	private:
		const static int MaxDataCount;
		//IO mqtt主题
		static const std::string EventTopic;


		SqliteWriter _writer;

		MqttChannel* _mqtt;

		std::mutex _mutex;

		std::queue<EventData> _datas;
		std::queue<EventData> _tempDatas;
	};

}

