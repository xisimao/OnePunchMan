#include "TrafficData.h"

using namespace std;
using namespace OnePunchMan;

string TrafficDirectory::TempDir("../temp/");
string TrafficDirectory::DataDir("../data/");
string TrafficDirectory::FileDir("../files/");
string TrafficDirectory::FileLink("files");
string TrafficDirectory::WebDir;

TrafficData::TrafficData(const std::string& dbName)
	:_dbName(dbName),_sqlite(dbName)
{

}

string TrafficData::LastError()
{
	return _sqlite.LastError();
}

TrafficChannel TrafficData::FillChannel(const SqliteReader& sqlite)
{
	TrafficChannel channel;
	channel.ChannelIndex = sqlite.GetInt(0);
	channel.ChannelName = sqlite.GetString(1);
	channel.ChannelUrl = sqlite.GetString(2);
	channel.ChannelType = sqlite.GetInt(3);
	channel.DeviceId = sqlite.GetString(4);

	channel.Loop = sqlite.GetInt(5);
	channel.OutputDetect = sqlite.GetInt(6);
	channel.OutputImage = sqlite.GetInt(7);
	channel.OutputReport = sqlite.GetInt(8);
	channel.OutputRecogn = sqlite.GetInt(9);
	channel.GlobalDetect = sqlite.GetInt(10);

	channel.LaneWidth = sqlite.GetDouble(11);
	channel.ReportProperties = sqlite.GetInt(12);

	SqliteReader flowLaneSqlite(_dbName);
	string flowLaneSql(StringEx::Combine("Select * From Flow_Lane Where ChannelIndex=", channel.ChannelIndex, " Order By LaneIndex"));
	if (flowLaneSqlite.BeginQuery(flowLaneSql))
	{
		while (flowLaneSqlite.HasRow())
		{
			FlowLane lane = FillFlowLane(flowLaneSqlite);
			channel.FlowLanes.push_back(lane);
		}
		flowLaneSqlite.EndQuery();
	}

	SqliteReader eventLaneSqlite(_dbName);
	string eventLaneSql(StringEx::Combine("Select * From Event_Lane Where ChannelIndex=", channel.ChannelIndex, " Order By LaneIndex"));
	if (eventLaneSqlite.BeginQuery(eventLaneSql))
	{
		while (eventLaneSqlite.HasRow())
		{
			EventLane lane = FillEventLane(eventLaneSqlite);
			channel.EventLanes.push_back(lane);
		}
		eventLaneSqlite.EndQuery();
	}

	return channel;
}

FlowLane TrafficData::FillFlowLane(const SqliteReader& sqlite)
{
	FlowLane lane;
	lane.ChannelIndex = sqlite.GetInt(0);
	lane.LaneId = sqlite.GetString(1);
	lane.LaneName = sqlite.GetString(2);
	lane.LaneIndex = sqlite.GetInt(3);
	lane.Direction = sqlite.GetInt(4);
	lane.FlowDirection = sqlite.GetInt(5);
	lane.StopLine = sqlite.GetString(6);
	lane.FlowDetectLine = sqlite.GetString(7);
	lane.QueueDetectLine = sqlite.GetString(8);
	lane.LaneLine1 = sqlite.GetString(9);
	lane.LaneLine2 = sqlite.GetString(10);
	lane.FlowRegion = sqlite.GetString(11);
	lane.QueueRegion = sqlite.GetString(12);
	lane.IOIp = sqlite.GetString(13);
	lane.IOPort = sqlite.GetInt(14);
	lane.IOIndex = sqlite.GetInt(15);
	lane.ReportProperties = sqlite.GetInt(16);
	return lane;
}

EventLane TrafficData::FillEventLane(const SqliteReader& sqlite)
{
	EventLane lane;
	lane.ChannelIndex = sqlite.GetInt(0);
	lane.LaneIndex = sqlite.GetInt(1);
	lane.LaneName = sqlite.GetString(2);
	lane.LaneType = sqlite.GetInt(3);
	lane.Region = sqlite.GetString(4);
	lane.Line = sqlite.GetString(5);
	return lane;
}

vector<TrafficChannel> TrafficData::GetChannels()
{
	vector<TrafficChannel> channels;
	SqliteReader sqlite(_dbName);
	if (sqlite.BeginQuery("Select * From System_Channel Order By ChannelIndex"))
	{
		while (sqlite.HasRow())
		{
			channels.push_back(FillChannel(sqlite));
		}
		sqlite.EndQuery();
	}
	return channels;
}

TrafficChannel TrafficData::GetChannel(int channelIndex)
{
	TrafficChannel channel;
	SqliteReader sqlite(_dbName);
	if (sqlite.BeginQuery(StringEx::Combine("Select * From System_Channel Where ChannelIndex=", channelIndex)))
	{
		if (sqlite.HasRow())
		{
			channel = FillChannel(sqlite);
		}
		sqlite.EndQuery();
	}
	return channel;
}

bool TrafficData::SetChannel(const TrafficChannel& channel)
{
	DeleteChannel(channel.ChannelIndex);
	return InsertChannel(channel);
}

bool TrafficData::SetChannels(const vector<TrafficChannel>& channels)
{
	ClearChannels();
	for (vector<TrafficChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (!InsertChannel(*it))
		{
			ClearChannels();
			return false;
		}
	}
	return true;
}

bool TrafficData::InsertChannel(const TrafficChannel& channel)
{
	string sql = StringEx::Combine("Insert Into System_Channel (ChannelIndex,ChannelName,ChannelUrl,ChannelType,Loop,OutputDetect,OutputImage,OutputReport,OutputRecogn,GlobalDetect,DeviceId,LaneWidth,ReportProperties) Values ("
		, channel.ChannelIndex, ","
		, "'", channel.ChannelName, "',"
		, "'", channel.ChannelUrl, "',"
		, channel.ChannelType, ","
		, channel.Loop, ","
		, channel.OutputDetect, ","
		, channel.OutputImage, ","
		, channel.OutputReport, ","
		, channel.OutputRecogn, ","
		, channel.GlobalDetect, ","
		, "'", channel.DeviceId, "',"
		, channel.LaneWidth, ","
		, channel.ReportProperties
		, ")");

	if (_sqlite.ExecuteRowCount(sql) == 1)
	{
		for (vector<FlowLane>::const_iterator it = channel.FlowLanes.begin(); it != channel.FlowLanes.end(); ++it)
		{
			string laneSql(StringEx::Combine("Insert Into Flow_Lane (ChannelIndex,LaneId,LaneName,LaneIndex,Direction,FlowDirection,StopLine,FlowDetectLine,QueueDetectLine,LaneLine1,LaneLine2,FlowRegion,QueueRegion,IOIp,IOPort,IOIndex) Values ("
				, it->ChannelIndex, ","
				, "'", it->LaneId, "',"
				, "'", it->LaneName, "',"
				, it->LaneIndex, ","
				, it->Direction, ","
				, it->FlowDirection, ","
				, "'", it->StopLine, "',"
				, "'", it->FlowDetectLine, "',"
				, "'", it->QueueDetectLine, "',"
				, "'", it->LaneLine1, "',"
				, "'", it->LaneLine2, "',"
				, "'", it->FlowRegion, "',"
				, "'", it->QueueRegion, "',"
				, "'", it->IOIp, "',"
				, it->IOPort, ","
				, it->IOIndex
				, ")"));
			if (_sqlite.ExecuteRowCount(laneSql) != 1)
			{
				return false;
			}
		}
		for (vector<EventLane>::const_iterator it = channel.EventLanes.begin(); it != channel.EventLanes.end(); ++it)
		{
			string laneSql(StringEx::Combine("Insert Into Event_Lane (ChannelIndex,LaneIndex,LaneName,LaneType,Region,Line) Values ("
				, it->ChannelIndex, ","
				, it->LaneIndex, ","
				, "'", it->LaneName, "',"
				, it->LaneType, ","
				, "'", it->Region, "',"
				, "'", it->Line, "'"
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

bool TrafficData::DeleteChannel(int channelIndex)
{
	int result = _sqlite.ExecuteRowCount(StringEx::Combine("Delete From System_Channel Where ChannelIndex=", channelIndex));
	if (result == 1)
	{
		_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Flow_Lane Where ChannelIndex=", channelIndex));
		_sqlite.ExecuteRowCount(StringEx::Combine("Delete From Event_Lane Where ChannelIndex=", channelIndex));
		return true;
	}
	return false;
}

void TrafficData::ClearChannels()
{
	_sqlite.ExecuteRowCount("Delete From System_Channel");
	_sqlite.ExecuteRowCount("Delete From Flow_Lane");
	_sqlite.ExecuteRowCount("Delete From Event_Lane");
}

vector<FlowLane> TrafficData::GetFlowLanes(int channelIndex, const std::string& laneId)
{
	vector<FlowLane> lanes;
	SqliteReader sqlite(_dbName);
	string sql = StringEx::Combine("Select l.*,c.ReportProperties From Flow_Lane As l Join System_Channel As c On l.ChannelIndex=c.ChannelIndex Where l.ChannelIndex!=", channelIndex, " And LaneId=='", laneId, "'");
	if (sqlite.BeginQuery(sql))
	{
		while (sqlite.HasRow())
		{
			lanes.push_back(FillFlowLane(sqlite));
		}
		sqlite.EndQuery();
	}
	return lanes;
}

string TrafficData::GetParameter(const string& key)
{
	string sql(StringEx::Combine("Select Value From System_Parameter Where Key='", key, "'"));
	SqliteReader sqlite(_dbName);
	string value;
	if (sqlite.BeginQuery(sql))
	{
		if (sqlite.HasRow())
		{
			value = sqlite.GetString(0);
		}
		sqlite.EndQuery();
	}
	return value;
}

bool TrafficData::SetParameter(const string& key, const string& value)
{
	string sql(StringEx::Combine("Update System_Parameter Set Value='", value, "' Where Key='", key, "'"));
	return _sqlite.ExecuteRowCount(sql) == 1;
}

GbParameter TrafficData::GetGbPrameter()
{
	string sql("Select * From GB_Config Limit 1");
	SqliteReader sqlite(_dbName);
	GbParameter parameter;
	if (sqlite.BeginQuery(sql))
	{
		if (sqlite.HasRow())
		{
			parameter.ServerIp = sqlite.GetString(1);
			parameter.ServerPort = sqlite.GetInt(2);
			parameter.SipPort = sqlite.GetInt(3);
			parameter.SipType = sqlite.GetInt(4);
			parameter.GbId = sqlite.GetString(5);
			parameter.DomainId = sqlite.GetString(6);
			parameter.UserName = sqlite.GetString(7);
			parameter.Password = sqlite.GetString(8);
		}
		sqlite.EndQuery();
	}
	return parameter;
}

bool TrafficData::SetGbPrameter(const GbParameter& parameter)
{
	_sqlite.ExecuteRowCount("Delete From GB_Config");
	string sql = StringEx::Combine("Insert Into GB_Config (ServerId,ServerIp,ServerPort,SipPort,SipType,GbId,DomainId,UserName,Password) Values ("
		, 1, ","
		, "'", parameter.ServerIp, "',"
		, "8000,"
		, parameter.SipPort, ","
		, parameter.SipType, ","
		, "'", parameter.GbId, "',"
		, "'", parameter.DomainId, "',"
		, "'", parameter.UserName, "',"
		, "'", parameter.Password, "')"
	);
	return _sqlite.ExecuteRowCount(sql) == 1;
}

vector<GbDevice> TrafficData::GetGbDevices()
{
	vector<GbDevice> devices;
	SqliteReader sqlite(_dbName);
	if (sqlite.BeginQuery("Select DeviceId,GbId,DeviceName,DeviceIp,DevicePort,UserName,Password From GB_Device"))
	{
		while (sqlite.HasRow())
		{
			GbDevice device;
			device.Id = sqlite.GetInt(0);
			device.DeviceId = sqlite.GetString(1);
			device.DeviceName = sqlite.GetString(2);
			device.DeviceIp = sqlite.GetString(3);
			device.DevicePort = sqlite.GetInt(4);
			device.UserName = sqlite.GetString(5);
			device.Password = sqlite.GetString(6);
			devices.push_back(device);
		}
		sqlite.EndQuery();
	}
	return devices;
}

int TrafficData::InsertGbDevice(const GbDevice& device)
{
	string sql = StringEx::Combine("Insert Into GB_Device (DeviceId,GbId,DeviceName,DeviceIp,DevicePort,UserName,Password) Values ("
		,"NULL,"
		, "'", device.DeviceId, "',"
		, "'", device.DeviceName, "',"
		, "'", device.DeviceIp, "',"
		, device.DevicePort, ","
		, "'", device.UserName, "',"
		, "'", device.Password, "')"
	);
	return _sqlite.ExecuteKey(sql);
}

bool TrafficData::UpdateGbDevice(const GbDevice& device)
{
	string sql = StringEx::Combine("Update GB_Device Set "
		, "GbId='", device.DeviceId, "',"
		, "DeviceName='", device.DeviceName, "',"
		, "DeviceIp='", device.DeviceIp, "',"
		, "DevicePort=", device.DevicePort, ","
		, "Username='", device.UserName, "',"
		, "Password='", device.Password, "' Where DeviceId="
		,device.Id
	);
	return _sqlite.ExecuteRowCount(sql) == 1;
}

bool TrafficData::DeleteGbDevice(int deviceId)
{
	string sql = StringEx::Combine("Delete From GB_Device Where DeviceId=", deviceId);
	return _sqlite.ExecuteRowCount(sql) == 1;
}

vector<GbChannel> TrafficData::GetGbChannels(const string& deviceId)
{
	string sql(StringEx::Combine("Select Id,ChannelId,ChannelName From GB_Channel Where DeviceId='", deviceId,"'"));
	SqliteReader sqlite(_dbName);
	vector<GbChannel> channels;
	if (sqlite.BeginQuery(sql))
	{
		if (sqlite.HasRow())
		{
			GbChannel channel;
			channel.Id = sqlite.GetInt(0);
			channel.ChannelId = sqlite.GetString(1);
			channel.ChannelName = sqlite.GetString(2);
			channels.push_back(channel);
		}
		sqlite.EndQuery();
	}
	return channels;
}

void TrafficData::UpdateDb()
{
	SetParameter("Version", "2.1.0");
	SetParameter("VersionValue", "20101");
}