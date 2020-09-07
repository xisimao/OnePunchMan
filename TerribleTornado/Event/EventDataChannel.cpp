#include "EventDataChannel.h"

using namespace std;
using namespace OnePunchMan;

const int EventDataChannel::MaxDataCount = 50;

const string EventDataChannel::EventTopic("Event");

EventDataChannel::EventDataChannel(MqttChannel* mqtt)
	:ThreadObject("event data"), _writer("event.db"), _mqtt(mqtt),_datas(),_tempDatas()
{

}

void EventDataChannel::Init()
{
	SqliteReader reader("event.db");
	if (reader.BeginQuery("Select * From Event_Data"))
	{
		while (reader.HasRow())
		{
			EventData data;
			data.Id = reader.GetInt(0);
			data.Guid = reader.GetString(1);
			data.ChannelUrl = reader.GetString(2);
			data.LaneIndex = reader.GetInt(3);
			data.TimeStamp = reader.GetLong(4);
			data.Type = reader.GetInt(5);
			_datas.push(data);
		}
		reader.EndQuery();
	}
}

void EventDataChannel::Push(const EventData& data)
{
	lock_guard<mutex> lck(_mutex);
	_tempDatas.push(data);
}

void EventDataChannel::StartCore()
{
	while (!_cancelled)
	{
		if (_tempDatas.empty())
		{
			this_thread::sleep_for(chrono::seconds(1));
		}
		else
		{
			unique_lock<mutex> lck(_mutex);
			EventData data= _tempDatas.front();
			_tempDatas.pop();
			lck.unlock();

			string insertSql = StringEx::Combine("Insert Into Event_Data (Id,Guid,ChannelUrl,LaneIndex,TimeStamp,Type) Values "
				, "(NULL,"
				, "'", data.Guid, "',"
				, "'", data.ChannelUrl, "',"
				, data.LaneIndex, ","
				, data.TimeStamp, ","
				, data.Type, ")");
			int id = _writer.ExecuteKey(insertSql);
			if (id != -1)
			{
				LogPool::Information(LogEvent::Event, "添加事件数据", data.Guid);
				data.Id = id;
				rename(data.GetTempImage(1).c_str(), data.GetImage(1).c_str());
				rename(data.GetTempImage(2).c_str(), data.GetImage(2).c_str());
				rename(data.GetTempVideo().c_str(), data.GetVideo().c_str());
				_datas.push(data);

				string json;
				JsonSerialization::SerializeValue(&json, "channelUrl", data.ChannelUrl);
				JsonSerialization::SerializeValue(&json, "laneIndex", data.LaneIndex);
				JsonSerialization::SerializeValue(&json, "timeStamp", data.TimeStamp);
				JsonSerialization::SerializeValue(&json, "type", data.Type);
				JsonSerialization::SerializeValue(&json, "image1",data.GetImageLink(1));
				JsonSerialization::SerializeValue(&json, "image2", data.GetImageLink(2));
				JsonSerialization::SerializeValue(&json, "video", data.GetVideo());
				if (_mqtt != NULL)
				{
					_mqtt->Send(EventTopic,json);
				}

				while (_datas.size() > MaxDataCount)
				{
					EventData removeData = _datas.front();
					_datas.pop();
					string deleteSql = StringEx::Combine("Delete From Event_Data Where Id=", removeData.Id);
					_writer.ExecuteRowCount(deleteSql);
					remove(removeData.GetImage(1).c_str());
					remove(removeData.GetImage(2).c_str());
					remove(removeData.GetVideo().c_str());
					LogPool::Information(LogEvent::Event, "删除添加事件数据", removeData.TimeStamp);
				}
			}
		}
	}
}