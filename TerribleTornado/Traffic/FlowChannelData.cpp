#include "FlowChannelData.h"

using namespace std;
using namespace Saitama;

vector<FlowChannel> FlowChannelData::GetList()
{
	vector<FlowChannel> channels;
	string channelSql("Select * From Flow_Channel");
	SqliteReader sqlite;
	if (sqlite.BeginQuery(channelSql))
	{
		while (sqlite.HasRow()) 
		{
			channels.push_back(GetChannel(sqlite));
		}
		sqlite.EndQuery();
	}
	return channels;
}

FlowChannel FlowChannelData::Get(int channelIndex)
{
	FlowChannel channel;
	string sql(StringEx::Combine("Select * From Flow_Channel Where ChannelIndex=", channelIndex));
	SqliteReader sqlite;
	if (sqlite.BeginQuery(sql))
	{
		if (sqlite.HasRow()) 
		{
			channel=GetChannel(sqlite);
		}
		sqlite.EndQuery();
	}
	return channel;
}


FlowChannel FlowChannelData::GetChannel(const SqliteReader& sqlite)
{
	FlowChannel channel;
	channel.ChannelIndex = sqlite.GetInteger(0);
	channel.ChannelName = sqlite.GetString(1);
	channel.ChannelUrl = sqlite.GetString(2);
	channel.ChannelType = sqlite.GetInteger(3);
	channel.RtspUser = sqlite.GetString(4);
	channel.RtspPassword = sqlite.GetString(5);
	channel.RtspProtocol = sqlite.GetInteger(6);
	channel.IsLoop = sqlite.GetInteger(7);

	SqliteReader laneSqlite;
	string laneSql(StringEx::Combine("Select * From Flow_Lane Where ChannelIndex=", channel.ChannelIndex));
	if (laneSqlite.BeginQuery(laneSql))
	{
		while (laneSqlite.HasRow())
		{
			Lane lane;
			lane.ChannelIndex = laneSqlite.GetInteger(0);
			lane.LaneId = laneSqlite.GetString(1);
			lane.LaneName = laneSqlite.GetString(2);
			lane.LaneIndex = laneSqlite.GetInteger(3);
			lane.LaneType = laneSqlite.GetInteger(4);
			lane.Direction = laneSqlite.GetInteger(5);
			lane.FlowDirection = laneSqlite.GetInteger(6);
			lane.Length = laneSqlite.GetInteger(7);
			lane.IOIp = laneSqlite.GetString(8);
			lane.IOPort = laneSqlite.GetInteger(9);
			lane.IOIndex = laneSqlite.GetInteger(10);
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
	string channelSql(StringEx::Combine("Insert Into Flow_Channel (ChannelIndex,ChannelName,ChannelUrl,ChannelType,RtspUser,RtspPassword,RtspProtocol,IsLoop) Values ("
		, channel.ChannelIndex, ","
		, "'", channel.ChannelName, "',"
		, "'", channel.ChannelUrl, "',"
		, channel.ChannelType, ","
		, "'", channel.RtspUser, "',"
		, "'", channel.RtspPassword, "',"
		, channel.RtspProtocol, ","
		, channel.IsLoop
		, ")"));
	if (_sqlite.ExecuteRowCount(channelSql))
	{
		for (vector<Lane>::const_iterator it = channel.Lanes.begin(); it != channel.Lanes.end(); ++it)
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
			_sqlite.ExecuteRowCount(laneSql);
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

void FlowChannelData::SetList(const vector<FlowChannel>& channels)
{
	_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Channel"));
	_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Lane"));

	for (vector<FlowChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		Insert(*it);
	}
}

int FlowChannelData::Delete(int channelIndex)
{
	int result = _sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Channel Where ChannelIndex=", channelIndex));
	if (result==-1)
	{
		return -1;
	}
	else
	{
		_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Lane Where ChannelIndex=", channelIndex));
		return result;
	}
}

