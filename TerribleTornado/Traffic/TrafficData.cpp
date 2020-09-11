#include "TrafficData.h"

using namespace std;
using namespace OnePunchMan;

string TrafficData::DbName("");

string TrafficDirectory::TempDir;
string TrafficDirectory::FileDir;
string TrafficDirectory::FileLink;

TrafficData::TrafficData()
	:_sqlite(DbName)
{

}

void TrafficData::Init(const std::string& dbName)
{
	DbName = dbName;
}

string TrafficData::LastError()
{
	return _sqlite.LastError();
}

void TrafficData::FillChannel(const SqliteReader& sqlite, TrafficChannel* channel)
{
	channel->ChannelIndex = sqlite.GetInt(0);
	channel->ChannelName = sqlite.GetString(1);
	channel->ChannelUrl = sqlite.GetString(2);
	channel->ChannelType = sqlite.GetInt(3);
	channel->Loop = sqlite.GetInt(4);
	channel->OutputReport = sqlite.GetInt(5);
	channel->OutputImage = sqlite.GetInt(6);
	channel->OutputRecogn = sqlite.GetInt(7);
	channel->GlobalDetect = sqlite.GetInt(8);
	channel->DeviceId = sqlite.GetString(9);
}
string TrafficData::GetChannelList()
{
	return string("Select * From System_Channel Order By ChannelIndex");
}

string TrafficData::GetChannel(int channelIndex)
{
	return StringEx::Combine("Select * From System_Channel Where ChannelIndex=", channelIndex);
}

string TrafficData::InsertChannel(const TrafficChannel* channel)
{
	return StringEx::Combine("Insert Into System_Channel (ChannelIndex,ChannelName,ChannelUrl,ChannelType,Loop,OutputImage,OutputReport,OutputRecogn,GlobalDetect,DeviceId) Values ("
		, channel->ChannelIndex, ","
		, "'", channel->ChannelName, "',"
		, "'", channel->ChannelUrl, "',"
		, channel->ChannelType, ","
		, channel->Loop, ","
		, channel->OutputImage, ","
		, channel->OutputReport, ","
		, channel->OutputRecogn, ","
		, channel->GlobalDetect, ","
		, "'",channel->DeviceId,"'"
		, ")");
}

string TrafficData::DeleteChannel(int channelIndex)
{
	return StringEx::Combine("Delete From System_Channel Where ChannelIndex=", channelIndex);
}

string TrafficData::ClearChannel()
{
	return string("Delete From System_Channel");
}

string TrafficData::GetParameter(const string& key)
{
	string sql(StringEx::Combine("Select Value From System_Parameter Where Key='", key, "'"));
	SqliteReader sqlite(DbName);
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
	SqliteReader sqlite(DbName);
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

vector<GbDevice> TrafficData::GetGbDeviceList()
{
	vector<GbDevice> devices;
	SqliteReader sqlite(DbName);
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

vector<GbChannel> TrafficData::GetGbChannelList(const string& deviceId)
{
	string sql(StringEx::Combine("Select Id,ChannelId,ChannelName From GB_Channel Where DeviceId='", deviceId,"'"));
	SqliteReader sqlite(DbName);
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
	SqliteReader sqlite(DbName);
	string sql("Select * From System_Parameter Limit 1");
	if (!sqlite.BeginQuery(sql))
	{
		_sqlite.ExecuteRowCount("CREATE TABLE [System_Parameter] ([Key] TEXT NOT NULL, [Value] TEXT NOT NULL, CONSTRAINT[PK_System_Parameter] PRIMARY KEY([Key]))");
		_sqlite.ExecuteRowCount("Insert Into System_Parameter (Key,Value) Values ('Version','')");
		_sqlite.ExecuteRowCount("Insert Into System_Parameter (Key,Value) Values ('VersionValue','')");
		_sqlite.ExecuteRowCount("Insert Into System_Parameter (Key,Value) Values ('SN','')");
	}

	sql= "Select * From GB_Config Limit 1";
	if (!sqlite.BeginQuery(sql))
	{
		_sqlite.ExecuteRowCount("CREATE TABLE[GB_Config]([ServerId] text NOT NULL, [ServerIp] text NOT NULL, [ServerPort] bigint NOT NULL, [SipPort] bigint NOT NULL, [SipType] bigint NOT NULL, [GbId] text NOT NULL, [DomainId] text NOT NULL, [UserName] text NOT NULL, [Password] text NOT NULL, CONSTRAINT[sqlite_autoindex_GB_Config] PRIMARY KEY([ServerId]));");
		_sqlite.ExecuteRowCount("CREATE TABLE[GB_Device]([DeviceId] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, [DeviceName] text NOT NULL, [DeviceIp] text NOT NULL, [DevicePort] bigint NOT NULL, [GbId] text NOT NULL, [UserName] text NOT NULL, [Password] text NOT NULL);");	
		_sqlite.ExecuteRowCount("CREATE TABLE[GB_Channel]([Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, [ChannelId] text NOT NULL, [ChannelName] text NOT NULL, [DeviceId] text NOT NULL);CREATE UNIQUE INDEX[GB_Channel_GB_Channel_ChannelId] ON[GB_Channel]([ChannelId] ASC);");	
	}

	sql = "Select * From System_Log Limit 1";
	if (!sqlite.BeginQuery(sql))
	{
		_sqlite.ExecuteRowCount("CREATE TABLE [System_Log] ([Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, [LogLevel] bigint NOT NULL, [LogEvent] bigint NOT NULL, [Time] text NOT NULL, [Content] text NOT NULL);");
	}

}