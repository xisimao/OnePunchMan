#include "FlowData.h"

using namespace std;
using namespace OnePunchMan;

vector<FlowChannel> FlowChannelData::GetList()
{
	vector<FlowChannel> channels;
	SqliteReader sqlite(DbName);
	if (sqlite.BeginQuery(GetChannelsSql()))
	{
		while (sqlite.HasRow()) 
		{
			channels.push_back(FillChannel(sqlite));
		}
		sqlite.EndQuery();
	}
	return channels;
}

vector<FlowLane> FlowChannelData::GetLaneList(int channelIndex, const std::string& laneId)
{
	vector<FlowLane> lanes;
	SqliteReader sqlite(DbName);
	string sql = StringEx::Combine("Select * From Flow_Lane Where ChannelIndex!=", channelIndex, " And LaneId=='", laneId, "'");
	if (sqlite.BeginQuery(sql))
	{
		while (sqlite.HasRow())
		{
			lanes.push_back(FillLane(sqlite));
		}
		sqlite.EndQuery();
	}
	return lanes;
}

FlowChannel FlowChannelData::Get(int channelIndex)
{
	FlowChannel channel;
	SqliteReader sqlite(DbName);
	if (sqlite.BeginQuery(GetChannelSql(channelIndex)))
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
	TrafficData::FillChannel(sqlite, &channel);

	SqliteReader laneSqlite(DbName);
	string laneSql(StringEx::Combine("Select * From Flow_Lane Where ChannelIndex=", channel.ChannelIndex," Order By LaneIndex"));
	if (laneSqlite.BeginQuery(laneSql))
	{
		while (laneSqlite.HasRow())
		{
			FlowLane lane = FillLane(laneSqlite);
			channel.Lanes.push_back(lane);
		}
		laneSqlite.EndQuery();
	}
	return channel;
}

FlowLane FlowChannelData::FillLane(const SqliteReader& sqlite)
{
	FlowLane lane;
	lane.ChannelIndex = sqlite.GetInt(0);
	lane.LaneId = sqlite.GetString(1);
	lane.LaneName = sqlite.GetString(2);
	lane.LaneIndex = sqlite.GetInt(3);
	lane.LaneType = sqlite.GetInt(4);
	lane.Direction = sqlite.GetInt(5);
	lane.FlowDirection = sqlite.GetInt(6);
	lane.Length = sqlite.GetInt(7);
	lane.IOIp = sqlite.GetString(8);
	lane.IOPort = sqlite.GetInt(9);
	lane.IOIndex = sqlite.GetInt(10);
	lane.DetectLine = sqlite.GetString(11);
	lane.StopLine = sqlite.GetString(12);
	lane.Region = sqlite.GetString(13);
	lane.ReportProperties = sqlite.GetInt(14);
	return lane;
}

bool FlowChannelData::InsertChannel(const FlowChannel& channel)
{
	if (_sqlite.ExecuteRowCount(InsertChannelSql(&channel))==1)
	{
		for (vector<FlowLane>::const_iterator it = channel.Lanes.begin(); it != channel.Lanes.end(); ++it)
		{
			if (!InsertLane(*it))
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

bool FlowChannelData::InsertLane(const FlowLane& lane)
{
	string laneSql(StringEx::Combine("Insert Into Flow_Lane (ChannelIndex,LaneId,LaneName,LaneIndex,LaneType,Direction,FlowDirection,Length,IOIp,IOPort,IOIndex,DetectLine,StopLine,Region,ReportProperties) Values ("
		, lane.ChannelIndex, ","
		, "'", lane.LaneId, "',"
		, "'", lane.LaneName, "',"
		, lane.LaneIndex, ","
		, lane.LaneType, ","
		, lane.Direction, ","
		, lane.FlowDirection, ","
		, lane.Length, ","
		, "'", lane.IOIp, "',"
		, lane.IOPort, ","
		, lane.IOIndex, ","
		, "'", lane.DetectLine, "',"
		, "'", lane.StopLine, "',"
		, "'", lane.Region, "',"
		, "'", lane.ReportProperties, "'"
		, ")"));
	return _sqlite.ExecuteRowCount(laneSql) == 1;
}

bool FlowChannelData::Set(const FlowChannel& channel)
{
	Delete(channel.ChannelIndex);
	return InsertChannel(channel);
}

bool FlowChannelData::SetList(const vector<FlowChannel>& channels)
{
	Clear();
	for (vector<FlowChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (!InsertChannel(*it))
		{
			Clear();
			return false;
		}
	}
	return true;
}

bool FlowChannelData::Delete(int channelIndex)
{
	int result = _sqlite.ExecuteRowCount(DeleteChannelSql(channelIndex));
	if (result==1)
	{
		_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Lane Where ChannelIndex=", channelIndex));
		return true;
	}
	return false;
}

void FlowChannelData::Clear()
{
	_sqlite.ExecuteRowCount(ClearChannelSql());
	_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Lane"));
}

void FlowChannelData::UpdateDb()
{
	TrafficData::UpdateDb();
	int versionValue = StringEx::Convert<int>(GetParameter("VersionValue"));
	if (versionValue < 20013)
	{
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN [Loop] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN [OutputReport] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN [OutputImage] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN [OutputRecogn] INTEGER NULL;");
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] ADD COLUMN [GlobalDetect] INTEGER NULL;");
	}
	if (versionValue < 20021)
	{
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Channel] RENAME TO [System_Channel];");
		_sqlite.ExecuteRowCount("ALTER TABLE[System_Channel] ADD COLUMN [DeviceId] TEXT NULL;");
	}
	if (versionValue < 20027)
	{
		SqliteReader laneSqlite(DbName);
		string laneSql("Select * From Flow_Lane");
		vector<FlowLane> lanes;
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
				lane.Region = laneSqlite.GetString(15);
				lanes.push_back(lane);
			}
			laneSqlite.EndQuery();
		}
		_sqlite.ExecuteRowCount("DROP TABLE [Flow_Lane];");
		_sqlite.ExecuteRowCount("CREATE TABLE [Flow_Lane] ([ChannelIndex] text NOT NULL, [LaneId] text NOT NULL, [LaneName] text NOT NULL, [LaneIndex] int NOT NULL, [LaneType] int NOT NULL, [Direction] int NOT NULL, [FlowDirection] int NOT NULL, [Length] int NOT NULL, [IOIp] text NOT NULL, [IOPort] int NOT NULL, [IOIndex] int NOT NULL, [DetectLine] text NULL, [StopLine] text NULL, [Region] text NULL, CONSTRAINT[sqlite_autoindex_Flow_Lane] PRIMARY KEY([ChannelIndex], [LaneId])); ");
		for (vector<FlowLane>::iterator it = lanes.begin(); it != lanes.end(); ++it)
		{
			string laneSql(StringEx::Combine("Insert Into Flow_Lane (ChannelIndex,LaneId,LaneName,LaneIndex,LaneType,Direction,FlowDirection,Length,IOIp,IOPort,IOIndex,DetectLine,StopLine,Region) Values ("
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
				, "'", it->Region, "'"
				, ")"));
			 _sqlite.ExecuteRowCount(laneSql);
		}
	}
	if (versionValue < 20029)
	{
		_sqlite.ExecuteRowCount("ALTER TABLE[Flow_Lane] ADD COLUMN [ReportProperties] INTEGER NULL;");
	}

	SetParameter("Version", "2.0.0.30");
	SetParameter("VersionValue", "20030");
}