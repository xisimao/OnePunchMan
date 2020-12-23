#include "EventDataChannel.h"

using namespace std;
using namespace OnePunchMan;

const int EventDataChannel::MaxDataCount = 50;

const string EventDataChannel::EventTopic("Event");

EventDataChannel::EventDataChannel(MqttChannel* mqtt)
	:ThreadObject("event data"), _writer("traffic.db"), _mqtt(mqtt),_datas(),_tempDatas()
{

}

vector<EventData> EventDataChannel::GetDatas(int channelIndex)
{
	SqliteReader reader("traffic.db");
	string sql = "Select * From Event_Data Where ";
	sql.append(channelIndex==0? "1=1" : StringEx::Combine("ChannelIndex=", channelIndex));
	sql.append(" Order By TimeStamp Desc");
	vector<EventData> datas;
	if (reader.BeginQuery(sql))
	{
		while (reader.HasRow())
		{
			EventData data;
			data.Id = reader.GetInt(0);
			data.ChannelIndex = reader.GetInt(1);
			data.LaneIndex = reader.GetInt(2);
			data.Guid = reader.GetString(3);		
			data.TimeStamp = reader.GetLong(4);
			data.Type = reader.GetInt(5);
			datas.push_back(data);
		}
		reader.EndQuery();
	}
	return datas;
}

void EventDataChannel::AddData(const EventData& data)
{
	lock_guard<mutex> lck(_mutex);
	_tempDatas.push(data);
}

void EventDataChannel::StartCore()
{
	vector<EventData> datas = GetDatas();
	for (vector<EventData>::iterator it = datas.begin(); it != datas.end(); ++it)
	{
		_datas.push(*it);
	}
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

			string insertSql = StringEx::Combine("Insert Into Event_Data (Id,Guid,ChannelIndex,LaneIndex,TimeStamp,Type) Values "
				, "(NULL,"
				, "'", data.Guid, "',"
				, data.ChannelIndex, ","
				, data.LaneIndex, ","
				, data.TimeStamp, ","
				, data.Type, ")");
			int id = _writer.ExecuteKey(insertSql);
			if (id != -1)
			{
				data.Id = id;
				rename(data.GetTempImage(1).c_str(), data.GetImage(1).c_str());
				rename(data.GetTempImage(2).c_str(), data.GetImage(2).c_str());
				rename(data.GetTempVideo().c_str(), data.GetVideo().c_str());
				_datas.push(data);

				string json;
				JsonSerialization::SerializeValue(&json, "channelIndex", data.ChannelIndex);
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
				}
			}
		}
	}
}