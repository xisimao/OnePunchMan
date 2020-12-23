#include "DataMergeMap.h"

using namespace std;
using namespace OnePunchMan;

const string DataMergeMap::FlowTopic("Flow");

DataMergeMap::DataMergeMap(MqttChannel* mqtt)
	:_mqtt(mqtt)
{

}

void DataMergeMap::PushData(const FlowReportData& data)
{
	lock_guard<mutex> lck(_mutex);
	map<string, FlowReportData>::iterator it = _datas.find(data.LaneId);
	//全新数据，有选择的存储
	if (it == _datas.end())
	{
		FlowReportData tempData;
		tempData.ChannelUrl = data.ChannelUrl;
		tempData.LaneId = data.LaneId;
		tempData.TimeStamp = data.TimeStamp;
		tempData.ReportProperties = data.ReportProperties;

		if (tempData.ReportProperties & 0x01)
		{
			tempData.Persons = data.Persons;
			tempData.Bikes = data.Bikes;
			tempData.Motorcycles = data.Motorcycles;
			tempData.Cars = data.Cars;
			tempData.Tricycles = data.Tricycles;
			tempData.Buss = data.Buss;
			tempData.Vans = data.Vans;
			tempData.Trucks = data.Trucks;

			tempData.Speed = data.Speed;
			tempData.HeadDistance = data.HeadDistance;
			tempData.HeadSpace = data.HeadSpace;
			tempData.TimeOccupancy = data.TimeOccupancy;
			tempData.TrafficStatus = data.TrafficStatus;
		}

		if (tempData.ReportProperties & 0x02)
		{
			tempData.QueueLength = data.QueueLength;
			tempData.SpaceOccupancy = data.SpaceOccupancy;
		}
		
		_datas.insert(pair<string, FlowReportData>(data.LaneId, tempData));
	}
	else
	{
		//合并数据，有选择的替换
		if (it->second.TimeStamp == data.TimeStamp)
		{
			if (data.ReportProperties & 0x01)
			{
				it->second.Persons = data.Persons;
				it->second.Bikes = data.Bikes;
				it->second.Motorcycles = data.Motorcycles;
				it->second.Cars = data.Cars;
				it->second.Tricycles = data.Tricycles;
				it->second.Buss = data.Buss;
				it->second.Vans = data.Vans;
				it->second.Trucks = data.Trucks;

				it->second.Speed = data.Speed;
				it->second.TimeOccupancy = data.TimeOccupancy;
				it->second.HeadDistance = data.HeadDistance;
				it->second.HeadSpace = data.HeadDistance;
				it->second.TrafficStatus = data.TrafficStatus;
			}
			if (data.ReportProperties & 0x02)
			{
				it->second.QueueLength = data.QueueLength;
				it->second.SpaceOccupancy = data.SpaceOccupancy;
			}
		}
		//新时间段数据，有选择的清空旧数据
		else if (it->second.TimeStamp < data.TimeStamp)
		{
			if (_mqtt != NULL
				&& data.TimeStamp-it->second.TimeStamp==60000)
			{
				string flowLanesJson;			
				JsonSerialization::AddClassItem(&flowLanesJson, it->second.ToMessageJson());
				_mqtt->Send(FlowTopic, flowLanesJson);
			}
			it->second.TimeStamp = data.TimeStamp;
			it->second.ReportProperties = data.ReportProperties;

			if (data.ReportProperties & 0x01)
			{
				it->second.Persons = data.Persons;
				it->second.Bikes = data.Bikes;
				it->second.Motorcycles = data.Motorcycles;
				it->second.Cars = data.Cars;
				it->second.Tricycles = data.Tricycles;
				it->second.Buss = data.Buss;
				it->second.Vans = data.Vans;
				it->second.Trucks = data.Trucks;

				it->second.Speed = data.Speed;
				it->second.TimeOccupancy = data.TimeOccupancy;
				it->second.HeadDistance = data.HeadDistance;
				it->second.HeadSpace = data.HeadDistance;
				it->second.TrafficStatus = data.TrafficStatus;
			}
			else
			{
				it->second.Persons = 0;
				it->second.Bikes = 0;
				it->second.Motorcycles = 0;
				it->second.Cars = 0;
				it->second.Tricycles = 0;
				it->second.Buss = 0;
				it->second.Vans = 0;
				it->second.Trucks = 0;

				it->second.Speed = 0;
				it->second.TimeOccupancy = 0;
				it->second.HeadDistance = 0;
				it->second.HeadSpace = 0;
				it->second.TrafficStatus = 0;
			}

			if (data.ReportProperties & 0x02)
			{
				it->second.QueueLength = data.QueueLength;
				it->second.SpaceOccupancy = data.SpaceOccupancy;
			}
			else
			{
				it->second.QueueLength = 0;
				it->second.SpaceOccupancy = 0;
			}
		}
		else
		{
			LogPool::Warning(LogEvent::Flow, "wrong merge timestamp", it->second.TimeStamp, data.TimeStamp);
		}
	}
}

