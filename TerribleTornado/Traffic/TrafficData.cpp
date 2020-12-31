#include "TrafficData.h"

using namespace std;
using namespace OnePunchMan;

const string TrafficDirectory::TempDir("../temp/");
const string TrafficDirectory::DataDir("../data/");
const string TrafficDirectory::FileDir("../files/");
const string TrafficDirectory::FileLink("files");
const string TrafficDirectory::WebConfigPath("../web_flow/static/config/base.js");

const int TrafficChannel::AllPropertiesFlag = 3;

TrafficData::TrafficData(const std::string& dbName)
	:_dbName(dbName),_sqlite(dbName)
{

}

string TrafficData::LastError()
{
	return _sqlite.LastError();
}

void TrafficData::UpdateDb()
{
	SetParameter("Version", "2.1.1");
	SetParameter("VersionValue", "20101");
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
	channel.FreeSpeed = sqlite.GetDouble(13);

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
	string sql = StringEx::Combine("Insert Into System_Channel (ChannelIndex,ChannelName,ChannelUrl,ChannelType,Loop,OutputDetect,OutputImage,OutputReport,OutputRecogn,GlobalDetect,DeviceId,LaneWidth,ReportProperties,FreeSpeed) Values ("
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
		, channel.ReportProperties, ","
		, channel.FreeSpeed
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

bool TrafficData::InsertFlowData(const FlowData& data)
{
	string sql = StringEx::Combine("Insert Into Flow_Data (ChannelUrl,LaneId,TimeStamp,Persons,Bikes,Motorcycles,Cars,Tricycles,Buss,Vans,Trucks,Speed,TotalDistance,MeterPerPixel,TotalTime,HeadDistance,TotalSpan,HeadSpace,TimeOccupancy,TotalInTime,QueueLength,SpaceOccupancy,TotalQueueLength,LaneLength,CountQueueLength,TrafficStatus,FreeSpeed) Values ("
		, "'", data.ChannelUrl, "',"
		, "'", data.LaneId, "',"
		, data.TimeStamp, ","
		, data.Persons, ","
		, data.Bikes, ","
		, data.Motorcycles, ","
		, data.Cars, ","
		, data.Tricycles, ","
		, data.Buss, ","
		, data.Vans, ","
		, data.Trucks, ","
		, data.Speed, ","
		, data.TotalDistance, ","
		, data.MeterPerPixel, ","
		, data.TotalTime, ","
		, data.HeadDistance, ","
		, data.TotalSpan, ","
		, data.HeadSpace, ","
		, data.TimeOccupancy, ","
		, data.TotalInTime, ","
		, data.QueueLength, ","
		, data.SpaceOccupancy, ","
		, data.TotalQueueLength, ","
		, data.LaneLength,","
		, data.CountQueueLength, ","
		, data.TrafficStatus, ","
		, data.FreeSpeed
		, ")");
	return _sqlite.ExecuteRowCount(sql) == 1;
}

vector<FlowData> TrafficData::GetFlowDatas(const string& channelUrl, const string& laneId, int pageNum, int pageSize)
{
	SqliteReader reader(_dbName);
	string sql = "Select * From Flow_Data Where";
	sql.append(channelUrl.empty() ? " 1=1" : StringEx::Combine(" ChannelUrl='", channelUrl,"'"));
	sql.append(laneId.empty() ? " And 1=1" : StringEx::Combine(" And LaneId='", laneId,"'"));
	sql.append(" Order By Id Desc");
	sql.append(pageNum == 0 || pageSize == 0 ? "" : StringEx::Combine(" Limit ", (pageNum - 1) * pageSize, ",", pageSize));
	vector<FlowData> datas;
	if (reader.BeginQuery(sql))
	{
		while (reader.HasRow())
		{
			FlowData data;
			data.Id = reader.GetInt(0);
			data.ChannelUrl = reader.GetString(1);
			data.LaneId = reader.GetString(2);
			data.TimeStamp = reader.GetLong(3);
			data.Persons = reader.GetInt(4);
			data.Bikes = reader.GetInt(5);
			data.Motorcycles = reader.GetInt(6);
			data.Cars = reader.GetInt(7);
			data.Tricycles = reader.GetInt(8);
			data.Buss = reader.GetInt(9);
			data.Vans = reader.GetInt(10);
			data.Trucks = reader.GetInt(11);
			data.Speed = reader.GetInt(12);
			data.TotalDistance = reader.GetInt(13);
			data.MeterPerPixel = reader.GetInt(14);
			data.TotalInTime = reader.GetInt(15);
			data.HeadDistance = reader.GetInt(16);
			data.TotalSpan = reader.GetInt(17);
			data.HeadSpace = reader.GetInt(18);
			data.TimeOccupancy = reader.GetInt(19);
			data.TotalInTime = reader.GetInt(20);
			data.QueueLength = reader.GetInt(21);
			data.SpaceOccupancy = reader.GetInt(22);
			data.TotalQueueLength = reader.GetInt(23);
			data.LaneLength = reader.GetInt(24);
			data.CountQueueLength = reader.GetInt(25);
			data.TrafficStatus = reader.GetInt(26);
			data.FreeSpeed = reader.GetInt(27);
			datas.push_back(data);
		}
		reader.EndQuery();
	}
	return datas;
}

void TrafficData::DeleteFlowDatas(int keeyDay)
{
	long long timeStamp = DateTime::Today().TimeStamp() - keeyDay * 24 * 60 * 60 * 1000;
	string sql = StringEx::Combine("Delete From Flow_Data Where TimeStamp<",timeStamp);
	_sqlite.ExecuteRowCount(sql);
}

bool TrafficData::InsertVehicleData(const VehicleData& data)
{
	string insertSql = StringEx::Combine("Insert Into Vehicle_Data (ChannelUrl,LaneId,TimeStamp,Guid,CarType,CarColor,CarBrand,PlateType,PlateNumber) Values "
		, "('", data.ChannelUrl, "',"
		, "'", data.LaneId, "',"
		, data.TimeStamp, ","
		, "'", data.Guid, "',"
		, data.CarType, ","
		, data.CarColor, ","
		, "'", data.CarBrand, "',"
		, data.PlateType, ","
		, "'", data.PlateNumber, "'"
		, ")");
	return _sqlite.ExecuteRowCount(insertSql)==1;
}

vector<VehicleData> TrafficData::GetVehicleDatas(const string& channelUrl, const string& laneId, int pageNum, int pageSize)
{
	SqliteReader reader(_dbName);
	string sql = "Select * From Vehicle_Data Where";
	sql.append(channelUrl.empty() ? " 1=1" : StringEx::Combine(" ChannelUrl='", channelUrl, "'"));
	sql.append(laneId.empty() ? " And 1=1" : StringEx::Combine(" And LaneId='", laneId, "'"));
	sql.append(" Order By Id Desc");
	sql.append(pageNum == 0 || pageSize == 0 ? "" : StringEx::Combine(" Limit ", (pageNum - 1) * pageSize, ",", pageSize));
	vector<VehicleData> datas;
	if (reader.BeginQuery(sql))
	{
		while (reader.HasRow())
		{
			VehicleData data;
			data.Id = reader.GetInt(0);
			data.ChannelUrl = reader.GetString(1);
			data.LaneId = reader.GetString(2);
			data.TimeStamp = reader.GetLong(3);
			data.Guid = reader.GetString(4);
			data.CarType = reader.GetInt(5);
			data.CarColor = reader.GetInt(6);
			data.CarBrand = reader.GetString(7);
			data.PlateType = reader.GetInt(8);
			data.PlateNumber = reader.GetString(9);
			datas.push_back(data);
		}
		reader.EndQuery();
	}
	return datas;
}

void TrafficData::DeleteVehicleDatas(int keepCount)
{
	while (true)
	{
		vector<VehicleData> datas = GetVehicleDatas("", "", 2, keepCount);
		if (datas.empty())
		{
			break;
		}
		for (vector<VehicleData>::iterator it = datas.begin(); it != datas.end(); ++it)
		{
			string deleteSql = StringEx::Combine("Delete From Vehicle_Data Where Id=", it->Id);
			_sqlite.ExecuteRowCount(deleteSql);
			remove(TrafficDirectory::GetRecognBmp(it->Guid).c_str());
		}
	}
}

bool TrafficData::InsertBikeData(const BikeData& data)
{
	string insertSql = StringEx::Combine("Insert Into Bike_Data (ChannelUrl,LaneId,TimeStamp,Guid,BikeType) Values "
		, "('", data.ChannelUrl, "',"
		, "'", data.LaneId, "',"
		, data.TimeStamp, ","
		, "'", data.Guid, "',"
		, data.BikeType 
		, ")");
	return _sqlite.ExecuteRowCount(insertSql) == 1;
}

vector<BikeData> TrafficData::GetBikeDatas(const string& channelUrl, const string& laneId, int pageNum, int pageSize)
{
	SqliteReader reader(_dbName);
	string sql = "Select * From Bike_Data Where";
	sql.append(channelUrl.empty() ? " 1=1" : StringEx::Combine(" ChannelUrl='", channelUrl, "'"));
	sql.append(laneId.empty() ? " And 1=1" : StringEx::Combine(" And LaneId='", laneId, "'"));
	sql.append(" Order By Id Desc");
	sql.append(pageNum == 0 || pageSize == 0 ? "" : StringEx::Combine(" Limit ", (pageNum - 1) * pageSize, ",", pageSize));
	vector<BikeData> datas;
	if (reader.BeginQuery(sql))
	{
		while (reader.HasRow())
		{
			BikeData data;
			data.Id = reader.GetInt(0);
			data.ChannelUrl = reader.GetString(1);
			data.LaneId = reader.GetString(2);
			data.TimeStamp = reader.GetLong(3);
			data.Guid = reader.GetString(4);
			data.BikeType = reader.GetInt(5);
			datas.push_back(data);
		}
		reader.EndQuery();
	}
	return datas;
}

void TrafficData::DeleteBikeDatas(int keepCount)
{
	while (true)
	{
		vector<BikeData> datas = GetBikeDatas("", "", 2, keepCount);
		if (datas.empty())
		{
			break;
		}
		for (vector<BikeData>::iterator it = datas.begin(); it != datas.end(); ++it)
		{
			string deleteSql = StringEx::Combine("Delete From Bike_Data Where Id=", it->Id);
			_sqlite.ExecuteRowCount(deleteSql);
			remove(TrafficDirectory::GetRecognBmp(it->Guid).c_str());
		}
	}
}

bool TrafficData::InsertPedestrainData(const PedestrainData& data)
{
	string insertSql = StringEx::Combine("Insert Into Pedestrain_Data (ChannelUrl,LaneId,TimeStamp,Image,Sex,Age,UpperColor) Values "
		, "('", data.ChannelUrl, "',"
		, "'", data.LaneId, "',"
		, data.TimeStamp, ","
		, "'", data.Guid, "',"
		, data.Sex, ","
		, data.Age, ","
		, data.UpperColor
		, ")");
	return _sqlite.ExecuteRowCount(insertSql) == 1;
}

vector<PedestrainData> TrafficData::GetPedestrainDatas(const string& channelUrl, const string& laneId, int pageNum, int pageSize)
{
	SqliteReader reader(_dbName);
	string sql = "Select * From Pedestrain_Data Where";
	sql.append(channelUrl.empty() ? " 1=1" : StringEx::Combine(" ChannelUrl='", channelUrl, "'"));
	sql.append(laneId.empty() ? " And 1=1" : StringEx::Combine(" And LaneId='", laneId, "'"));
	sql.append(" Order By Id Desc");
	sql.append(pageNum == 0 || pageSize == 0 ? "" : StringEx::Combine(" Limit ", (pageNum - 1) * pageSize, ",", pageSize));
	vector<PedestrainData> datas;
	if (reader.BeginQuery(sql))
	{
		while (reader.HasRow())
		{
			PedestrainData data;
			data.Id = reader.GetInt(0);
			data.ChannelUrl = reader.GetString(1);
			data.LaneId = reader.GetString(2);
			data.TimeStamp = reader.GetLong(3);
			data.Guid = reader.GetString(4);
			data.Sex = reader.GetInt(5);
			data.Age = reader.GetInt(6);
			data.UpperColor = reader.GetInt(7);
			datas.push_back(data);
		}
		reader.EndQuery();
	}
	return datas;
}

void TrafficData::DeletePedestrainDatas(int keepCount)
{
	while (true)
	{
		vector<PedestrainData> datas = GetPedestrainDatas("", "", 2, keepCount);
		if (datas.empty())
		{
			break;
		}
		for (vector<PedestrainData>::iterator it = datas.begin(); it != datas.end(); ++it)
		{
			string deleteSql = StringEx::Combine("Delete From Pedestrain_Data Where Id=", it->Id);
			_sqlite.ExecuteRowCount(deleteSql);
			remove(TrafficDirectory::GetRecognBmp(it->Guid).c_str());
		}
	}
}

bool TrafficData::InsertEventData(const EventData& data)
{
	string insertSql = StringEx::Combine("Insert Into Event_Data (Guid,ChannelIndex,LaneIndex,TimeStamp,Type) Values "
		, "('", data.Guid, "',"
		, data.ChannelIndex, ","
		, data.LaneIndex, ","
		, data.TimeStamp, ","
		, data.Type, ")");
	return _sqlite.ExecuteRowCount(insertSql)==1;
}

vector<EventData> TrafficData::GetEventDatas(int channelIndex, int pageNum, int pageSize)
{
	SqliteReader reader(_dbName);
	string sql = "Select * From Event_Data";
	sql.append(channelIndex == 0 ? "" : StringEx::Combine(" Where ChannelIndex=", channelIndex));
	sql.append(" Order By Id Desc");
	sql.append(pageNum==0||pageSize==0?"":StringEx::Combine(" Limit ",(pageNum-1)*pageSize,",",pageSize));
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

void TrafficData::DeleteEventData(int keepCount)
{
	while (true)
	{
		vector<EventData> datas = GetEventDatas(0, 2, keepCount);
		if (datas.empty())
		{
			break;
		}
		for (vector<EventData>::iterator it = datas.begin(); it != datas.end(); ++it)
		{
			string deleteSql = StringEx::Combine("Delete From Event_Data Where Id=", it->Id);
			_sqlite.ExecuteRowCount(deleteSql);
			remove(TrafficDirectory::GetEventJpg(it->Guid, 1).c_str());
			remove(TrafficDirectory::GetEventJpg(it->Guid, 2).c_str());
			remove(TrafficDirectory::GetEventMp4(it->Guid).c_str());
		}
	}
}

