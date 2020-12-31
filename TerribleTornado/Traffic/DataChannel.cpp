#include "DataChannel.h"

using namespace std;
using namespace OnePunchMan;

const int DataChannel::MaxDataCount = 100;

DataChannel::DataChannel()
	:ThreadObject("data")
{

}

void DataChannel::PushEventData(const EventData& data)
{
	lock_guard<mutex> lck(_eventMutex);
	_eventDatas.push_back(data);
}

void DataChannel::PushFlowData(const FlowData& data)
{
	lock_guard<mutex> lck(_flowMutex);
	_flowDatas.push_back(data);
}

void DataChannel::PushVehicleData(const VehicleData& data)
{
	lock_guard<mutex> lck(_vehicleMutex);
	_vehicleDatas.push_back(data);
}

void DataChannel::PushBikeData(const BikeData& data)
{
	lock_guard<mutex> lck(_bikeMutex);
	_bikeDatas.push_back(data);
}

void DataChannel::PushPedestrainData(const PedestrainData& data)
{
	lock_guard<mutex> lck(_pedestrainMutex);
	_pedestrainDatas.push_back(data);
}

void DataChannel::StartCore()
{
	while (!_cancelled)
	{
		this_thread::sleep_for(chrono::minutes(1));
		TrafficData data;
		//流量数据
		unique_lock<mutex> flowLock(_flowMutex);
		vector<FlowData> flowDatas(_flowDatas.begin(), _flowDatas.end());
		_flowDatas.clear();
		flowLock.unlock();
		for (vector<FlowData>::iterator fit = flowDatas.begin(); fit != flowDatas.end(); ++fit)
		{
			if (fit->ReportProperties == TrafficChannel::AllPropertiesFlag)
			{
				data.InsertFlowData(*fit);
			}
			else
			{
				map<string, FlowData>::iterator it = _mergeFlowDatas.find(fit->LaneId);
				//全新数据，有选择的存储
				if (it == _mergeFlowDatas.end())
				{
					FlowData tempData;
					tempData.ChannelUrl =fit->ChannelUrl;
					tempData.LaneId =fit->LaneId;
					tempData.TimeStamp =fit->TimeStamp;
					tempData.ReportProperties =fit->ReportProperties;

					if (tempData.ReportProperties & 0x01)
					{
						tempData.Persons =fit->Persons;
						tempData.Bikes =fit->Bikes;
						tempData.Motorcycles =fit->Motorcycles;
						tempData.Cars =fit->Cars;
						tempData.Tricycles =fit->Tricycles;
						tempData.Buss =fit->Buss;
						tempData.Vans =fit->Vans;
						tempData.Trucks =fit->Trucks;

						tempData.Speed =fit->Speed;
						tempData.HeadDistance =fit->HeadDistance;
						tempData.HeadSpace =fit->HeadSpace;
						tempData.TimeOccupancy =fit->TimeOccupancy;
						tempData.TrafficStatus =fit->TrafficStatus;
					}

					if (tempData.ReportProperties & 0x02)
					{
						tempData.QueueLength =fit->QueueLength;
						tempData.SpaceOccupancy =fit->SpaceOccupancy;
					}

					_mergeFlowDatas.insert(pair<string, FlowData>(fit->LaneId, tempData));
				}
				else
				{
					//合并数据，有选择的替换
					if (it->second.TimeStamp ==fit->TimeStamp)
					{
						if (fit->ReportProperties & 0x01)
						{
							it->second.Persons =fit->Persons;
							it->second.Bikes =fit->Bikes;
							it->second.Motorcycles =fit->Motorcycles;
							it->second.Cars =fit->Cars;
							it->second.Tricycles =fit->Tricycles;
							it->second.Buss =fit->Buss;
							it->second.Vans =fit->Vans;
							it->second.Trucks =fit->Trucks;

							it->second.Speed =fit->Speed;
							it->second.TimeOccupancy =fit->TimeOccupancy;
							it->second.HeadDistance =fit->HeadDistance;
							it->second.HeadSpace =fit->HeadDistance;
							it->second.TrafficStatus =fit->TrafficStatus;
						}
						if (fit->ReportProperties & 0x02)
						{
							it->second.QueueLength =fit->QueueLength;
							it->second.SpaceOccupancy =fit->SpaceOccupancy;
						}
					}
					//新时间段数据，有选择的清空旧数据
					else if (it->second.TimeStamp <fit->TimeStamp)
					{
						if (fit->TimeStamp - it->second.TimeStamp == 60000)
						{
							data.InsertFlowData(it->second);
						}
						it->second.TimeStamp =fit->TimeStamp;
						it->second.ReportProperties =fit->ReportProperties;

						if (fit->ReportProperties & 0x01)
						{
							it->second.Persons =fit->Persons;
							it->second.Bikes =fit->Bikes;
							it->second.Motorcycles =fit->Motorcycles;
							it->second.Cars =fit->Cars;
							it->second.Tricycles =fit->Tricycles;
							it->second.Buss =fit->Buss;
							it->second.Vans =fit->Vans;
							it->second.Trucks =fit->Trucks;

							it->second.Speed =fit->Speed;
							it->second.TimeOccupancy =fit->TimeOccupancy;
							it->second.HeadDistance =fit->HeadDistance;
							it->second.HeadSpace =fit->HeadDistance;
							it->second.TrafficStatus =fit->TrafficStatus;
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

						if (fit->ReportProperties & 0x02)
						{
							it->second.QueueLength =fit->QueueLength;
							it->second.SpaceOccupancy =fit->SpaceOccupancy;
						}
						else
						{
							it->second.QueueLength = 0;
							it->second.SpaceOccupancy = 0;
						}
					}
				}
			}
		}

		unique_lock<mutex> vehicleLock(_vehicleMutex);
		vector<VehicleData> vehicleDatas(_vehicleDatas.begin(), _vehicleDatas.end());
		_vehicleDatas.clear();
		vehicleLock.unlock();
		for (vector<VehicleData>::iterator it = vehicleDatas.begin(); it != vehicleDatas.end(); ++it)
		{
			data.InsertVehicleData(*it);
		}
		data.DeleteVehicleDatas(MaxDataCount);

		unique_lock<mutex> bikeLock(_bikeMutex);
		vector<BikeData> bikeDatas(_bikeDatas.begin(), _bikeDatas.end());
		_bikeDatas.clear();
		bikeLock.unlock();
		for (vector<BikeData>::iterator it = bikeDatas.begin(); it != bikeDatas.end(); ++it)
		{
			data.InsertBikeData(*it);
		}
		data.DeleteBikeDatas(MaxDataCount);

		unique_lock<mutex> pedestrainMutex(_pedestrainMutex);
		vector<PedestrainData> pedestrainDatas(_pedestrainDatas.begin(), _pedestrainDatas.end());
		_pedestrainDatas.clear();
		pedestrainMutex.unlock();
		for (vector<PedestrainData>::iterator it = pedestrainDatas.begin(); it != pedestrainDatas.end(); ++it)
		{
			data.InsertPedestrainData(*it);
		}
		data.DeletePedestrainDatas(MaxDataCount);

		unique_lock<mutex> eventLock(_eventMutex);
		vector<EventData> eventDatas(_eventDatas.begin(), _eventDatas.end());
		_eventDatas.clear();
		eventLock.unlock();
		for (vector<EventData>::iterator it = eventDatas.begin(); it != eventDatas.end(); ++it)
		{
			data.InsertEventData(*it);
		}
		data.DeleteEventData(MaxDataCount);
	}
}