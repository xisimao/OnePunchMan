#include "EventData.h"

using namespace std;
using namespace OnePunchMan;

const string EventChannelData::DbName("event.db");

EventChannelData::EventChannelData()
	:_sqlite(DbName)
{

}

vector<EventChannel> EventChannelData::GetList()
{
	vector<EventChannel> channels;
	string channelSql("Select * From Event_Channel Order By ChannelIndex");
	SqliteReader sqlite(DbName);
	if (sqlite.BeginQuery(channelSql))
	{
		while (sqlite.HasRow()) 
		{
			channels.push_back(FillChannel(sqlite));
		}
		sqlite.EndQuery();
	}
	return channels;
}

EventChannel EventChannelData::Get(int channelIndex)
{
	EventChannel channel;
	string sql(StringEx::Combine("Select * From Event_Channel Where ChannelIndex=", channelIndex));
	SqliteReader sqlite(DbName);
	if (sqlite.BeginQuery(sql))
	{
		if (sqlite.HasRow()) 
		{
			channel=FillChannel(sqlite);
		}
		sqlite.EndQuery();
	}
	return channel;
}

EventChannel EventChannelData::FillChannel(const SqliteReader& sqlite)
{
	EventChannel channel;
	channel.ChannelIndex = sqlite.GetInt(0);
	channel.ChannelName = sqlite.GetString(1);
	channel.ChannelUrl = sqlite.GetString(2);
	channel.ChannelType = sqlite.GetInt(3);

	SqliteReader laneSqlite(DbName);
	string laneSql(StringEx::Combine("Select * From Event_Lane Where ChannelIndex=", channel.ChannelIndex," Order By LaneIndex"));
	if (laneSqlite.BeginQuery(laneSql))
	{
		while (laneSqlite.HasRow())
		{
			EventLane lane;
			lane.ChannelIndex = laneSqlite.GetInt(0);
			lane.LaneId = laneSqlite.GetString(1);
			lane.LaneName = laneSqlite.GetString(2);
			lane.LaneIndex = laneSqlite.GetInt(3);
			lane.Direction = laneSqlite.GetInt(4);
			lane.LaneType = laneSqlite.GetInt(5);
			lane.Region = laneSqlite.GetString(6);
			lane.DetectLine = laneSqlite.GetString(7);
			lane.StopLine = laneSqlite.GetString(8);
			lane.LaneLine1 = laneSqlite.GetString(9);
			lane.LaneLine2 = laneSqlite.GetString(10);
			channel.Lanes.push_back(lane);
		}
		laneSqlite.EndQuery();
	}
	return channel;
}

bool EventChannelData::Insert(const EventChannel& channel)
{
	string channelSql(StringEx::Combine("Insert Into Event_Channel (ChannelIndex,ChannelName,ChannelUrl,ChannelType) Values ("
		, channel.ChannelIndex, ","
		, "'", channel.ChannelName, "',"
		, "'", channel.ChannelUrl, "',"
		, channel.ChannelType
		, ")"));
	if (_sqlite.ExecuteRowCount(channelSql)==1)
	{
		for (vector<EventLane>::const_iterator it = channel.Lanes.begin(); it != channel.Lanes.end(); ++it)
		{
			string laneSql(StringEx::Combine("Insert Into Event_Lane (ChannelIndex,LaneId,LaneName,LaneIndex,Direction,LaneType,Region,DetectLine,StopLine,LaneLine1,Laneline2) Values ("
				, it->ChannelIndex, ","
				, "'", it->LaneId, "',"
				, "'", it->LaneName, "',"
				, it->LaneIndex, ","
				, it->Direction, ","
				, it->LaneType, ","
				, "'", it->Region, "',"
				, "'", it->DetectLine, "',"
				, "'", it->StopLine, "',"
				, "'", it->LaneLine1, "',"
				, "'", it->LaneLine2, "'"
				, ")"));
			if (_sqlite.ExecuteRowCount(laneSql) != 1)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool EventChannelData::Set(const EventChannel& channel)
{
	Delete(channel.ChannelIndex);
	return Insert(channel);
}

bool EventChannelData::SetList(const vector<EventChannel>& channels)
{
	Clear();
	for (vector<EventChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (!Insert(*it))
		{
			Clear();
			return false;
		}
	}
	return true;
}

bool EventChannelData::Delete(int channelIndex)
{
	int result = _sqlite.ExecuteRowCount(StringEx::Combine("Delete From Event_Channel Where ChannelIndex=", channelIndex));
	if (result==1)
	{
		_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Event_Lane Where ChannelIndex=", channelIndex));
		return true;
	}
	return false;
}

void EventChannelData::Clear()
{
	_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Event_Channel"));
	_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Event_Lane"));
}

string EventChannelData::LastError()
{
	return _sqlite.LastError();
}