#include "FlowData.h"

using namespace std;
using namespace OnePunchMan;

const string FlowChannelData::DbName("flow.db");

FlowChannelData::FlowChannelData()
	:_sqlite(DbName)
{

}

vector<FlowChannel> FlowChannelData::GetList()
{
	vector<FlowChannel> channels;
	string channelSql("Select * From Flow_Channel Order By ChannelIndex");
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

FlowChannel FlowChannelData::Get(int channelIndex)
{
	FlowChannel channel;
	string sql(StringEx::Combine("Select * From Flow_Channel Where ChannelIndex=", channelIndex));
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

FlowChannel FlowChannelData::FillChannel(const SqliteReader& sqlite)
{
	FlowChannel channel;
	channel.ChannelIndex = sqlite.GetInt(0);
	channel.ChannelName = sqlite.GetString(1);
	channel.ChannelUrl = sqlite.GetString(2);
	channel.ChannelType = sqlite.GetInt(3);

	SqliteReader laneSqlite(DbName);
	string laneSql(StringEx::Combine("Select * From Flow_Lane Where ChannelIndex=", channel.ChannelIndex));
	if (laneSqlite.BeginQuery(laneSql))
	{
		while (laneSqlite.HasRow())
		{
			FlowLane lane;
			lane.ChannelIndex = laneSqlite.GetInt(0);
			lane.LaneId = laneSqlite.GetString(1);
			lane.LaneName = laneSqlite.GetString(2);
			lane.LaneIndex = laneSqlite.GetInt(3);
			lane.LaneType = laneSqlite.GetInt(4);
			lane.Direction = laneSqlite.GetInt(5);
			lane.FlowDirection = laneSqlite.GetInt(6);
			lane.Length = laneSqlite.GetInt(7);
			lane.IOIp = laneSqlite.GetString(8);
			lane.IOPort = laneSqlite.GetInt(9);
			lane.IOIndex = laneSqlite.GetInt(10);
			lane.DetectLine = laneSqlite.GetString(11);
			lane.StopLine = laneSqlite.GetString(12);
			lane.LaneLine1 = laneSqlite.GetString(13);
			lane.LaneLine2 = laneSqlite.GetString(14);
			lane.Region = laneSqlite.GetString(15);
			channel.Lanes.push_back(lane);
		}
		laneSqlite.EndQuery();
	}
	return channel;
}

bool FlowChannelData::Insert(const FlowChannel& channel)
{
	string channelSql(StringEx::Combine("Insert Into Flow_Channel (ChannelIndex,ChannelName,ChannelUrl,ChannelType) Values ("
		, channel.ChannelIndex, ","
		, "'", channel.ChannelName, "',"
		, "'", channel.ChannelUrl, "',"
		, channel.ChannelType
		, ")"));
	if (_sqlite.ExecuteRowCount(channelSql)==1)
	{
		for (vector<FlowLane>::const_iterator it = channel.Lanes.begin(); it != channel.Lanes.end(); ++it)
		{
			string laneSql(StringEx::Combine("Insert Into Flow_Lane (ChannelIndex,LaneId,LaneName,LaneIndex,LaneType,Direction,FlowDirection,Length,IOIp,IOPort,IOIndex,DetectLine,StopLine,LaneLine1,LaneLine2,Region) Values ("
				, it->ChannelIndex, ","
				, "'", it->LaneId, "',"
				, "'", it->LaneName, "',"
				, it->LaneIndex, ","
				, it->LaneType, ","
				, it->Direction, ","
				, it->FlowDirection, ","
				, it->Length, ","
				, "'", it->IOIp, "',"
				, it->IOPort, ","
				, it->IOIndex, ","
				, "'", it->DetectLine, "',"
				, "'", it->StopLine, "',"
				, "'", it->LaneLine1, "',"
				, "'", it->LaneLine2, "',"
				, "'", it->Region, "'"
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

bool FlowChannelData::Set(const FlowChannel& channel)
{
	Delete(channel.ChannelIndex);
	return Insert(channel);
}

bool FlowChannelData::SetList(const vector<FlowChannel>& channels)
{
	Clear();
	for (vector<FlowChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (!Insert(*it))
		{
			Clear();
			return false;
		}
	}
	return true;
}

bool FlowChannelData::Delete(int channelIndex)
{
	int result = _sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Channel Where ChannelIndex=", channelIndex));
	if (result==1)
	{
		_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Lane Where ChannelIndex=", channelIndex));
		return true;
	}
	return false;
}

void FlowChannelData::Clear()
{
	_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Channel"));
	_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Lane"));
}

string FlowChannelData::LastError()
{
	return _sqlite.LastError();
}