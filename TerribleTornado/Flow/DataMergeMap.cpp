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
	if (it == _datas.end())
	{
		_datas.insert(pair<string, FlowReportData>(data.LaneId, data));
	}
	else
	{
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
		else if (it->second.TimeStamp < data.TimeStamp)
		{
			LogPool::Information(LogEvent::Flow, "merge flow->channel url:", it->second.ChannelUrl, "lane:", it->second.LaneId, "timestamp:", DateTime::ParseTimeStamp(it->second.TimeStamp).ToString(), "cars:", it->second.Cars, "tricycles:", it->second.Tricycles, "buss", it->second.Buss, "vans:", it->second.Vans, "trucks:", it->second.Trucks, "bikes:", it->second.Bikes, "motorcycles:", it->second.Motorcycles, "persons:", it->second.Persons, "speed(km/h):", it->second.Speed, "head distance(sec):", it->second.HeadDistance, "head space(m)", it->second.HeadSpace, "time occ(%):", it->second.TimeOccupancy, "traffic status:", it->second.TrafficStatus, "queue length(m):", it->second.QueueLength, "space occ(%):", it->second.SpaceOccupancy);
			
			if (_mqtt != NULL
				&&data.TimeStamp-it->second.TimeStamp==60000)
			{
				string flowLanesJson;
				string flowLaneJson;
				JsonSerialization::SerializeValue(&flowLaneJson, "channelUrl", it->second.ChannelUrl);
				JsonSerialization::SerializeValue(&flowLaneJson, "laneId", it->second.LaneId);
				JsonSerialization::SerializeValue(&flowLaneJson, "timeStamp", it->second.TimeStamp);

				JsonSerialization::SerializeValue(&flowLaneJson, "persons", it->second.Persons);
				JsonSerialization::SerializeValue(&flowLaneJson, "bikes", it->second.Bikes);
				JsonSerialization::SerializeValue(&flowLaneJson, "motorcycles", it->second.Motorcycles);
				JsonSerialization::SerializeValue(&flowLaneJson, "cars", it->second.Cars);
				JsonSerialization::SerializeValue(&flowLaneJson, "tricycles", it->second.Tricycles);
				JsonSerialization::SerializeValue(&flowLaneJson, "buss", it->second.Buss);
				JsonSerialization::SerializeValue(&flowLaneJson, "vans", it->second.Vans);
				JsonSerialization::SerializeValue(&flowLaneJson, "trucks", it->second.Trucks);
				JsonSerialization::SerializeValue(&flowLaneJson, "averageSpeed", static_cast<int>(it->second.Speed));
				JsonSerialization::SerializeValue(&flowLaneJson, "headDistance", it->second.HeadDistance);
				JsonSerialization::SerializeValue(&flowLaneJson, "headSpace", it->second.HeadSpace);
				JsonSerialization::SerializeValue(&flowLaneJson, "timeOccupancy", static_cast<int>(it->second.TimeOccupancy));
				JsonSerialization::SerializeValue(&flowLaneJson, "trafficStatus", it->second.TrafficStatus);
				JsonSerialization::SerializeValue(&flowLaneJson, "queueLength", it->second.QueueLength);
				JsonSerialization::SerializeValue(&flowLaneJson, "spaceOccupancy", it->second.SpaceOccupancy);
				JsonSerialization::AddClassItem(&flowLanesJson, flowLaneJson);
				_mqtt->Send(FlowTopic, flowLanesJson);
			}
			it->second.TimeStamp = data.TimeStamp;
			it->second.ReportProperties = data.ReportProperties;

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

			it->second.QueueLength = data.QueueLength;
			it->second.SpaceOccupancy = data.SpaceOccupancy;
		}
		else
		{
			LogPool::Warning(LogEvent::Flow, "wrong merge timestamp", it->second.TimeStamp, data.TimeStamp);
		}
	}
}

