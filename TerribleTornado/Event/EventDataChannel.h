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

	//事件数据操作线程
	class EventDataChannel:public ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: mqtt mqtt
		*/
		EventDataChannel(MqttChannel* mqtt);

		/**
		* @brief: 构造函数
		* @param: channelIndex 视频序号
		* @return: 事件数据集合
		*/
		static std::vector<EventData> GetDatas(int channelIndex=0);

		/**
		* @brief: 添加新的事件数据
		* @param: width 图片宽度
		* @param: height 图片高度
		* @param: mqtt mqtt
		* @param: encodeChannel 编码线程
		* @param: dataChannel 数据线程
		*/
		void AddData(const EventData& data);

	protected:
		void StartCore();

	private:
		//最多保存的事件数据数量
		const static int MaxDataCount;
		//IO mqtt主题
		static const std::string EventTopic;
		//数据库写入
		SqliteWriter _writer;
		//mqtt
		MqttChannel* _mqtt;
		//事件队列同步锁
		std::mutex _mutex;
		//当前保存的数据队列
		std::queue<EventData> _datas;
		//当前要添加的数据队列
		std::queue<EventData> _tempDatas;
	};

}

