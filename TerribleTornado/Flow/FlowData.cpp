#include "FlowData.h"

using namespace std;
using namespace OnePunchMan;

vector<FlowChannel> FlowChannelData::GetList()
{
	vector<FlowChannel> channels;
	string channelSql("Select * From Flow_Channel Order By ChannelIndex");
	SqliteReader sqlite(_dbName);
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
	SqliteReader sqlite(_dbName);
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
	channel.Loop = sqlite.GetInt(4);
	channel.OutputReport = sqlite.GetInt(5);
	channel.OutputImage = sqlite.GetInt(6);
	channel.OutputRecogn = sqlite.GetInt(7);
	channel.GlobalDetect = sqlite.GetInt(8);

	SqliteReader laneSqlite(_dbName);
	string laneSql(StringEx::Combine("Select * From Flow_Lane Where ChannelIndex=", channel.ChannelIndex," Order By LaneIndex"));
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
	string channelSql(StringEx::Combine("Insert Into Flow_Channel (ChannelIndex,ChannelName,ChannelUrl,ChannelType,Loop,OutputImage,OutputReport,OutputRecogn,GlobalDetect) Values ("
		, channel.ChannelIndex, ","
		, "'", channel.ChannelName, "',"
		, "'", channel.ChannelUrl, "',"
		, channel.ChannelType, ","
		, channel.Loop, ","
		, channel.OutputImage, ","
		, channel.OutputReport, ","
		, channel.OutputRecogn, ","
		, channel.GlobalDetect
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

void FlowChannelData::UpdateDb()
{
	TrafficData::UpdateDb();
	int versionValue = StringEx::Convert<int>(GetParameter("VersionValue"));
	if (versionValue < 20013)
	{
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN[Loop] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN[OutputReport] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN[OutputImage] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN[OutputRecogn] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN[GlobalDetect] INTEGER NULL;");
	}
	SetParameter("Version", "2.0.0.16");
	SetParameter("VersionValue", "20016");
}