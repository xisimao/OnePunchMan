#include "VideoChannel.h"

using namespace std;
using namespace Saitama;

VideoChannel::VideoChannel()
	:ThreadObject("video")
{

}

VideoChannel::~VideoChannel()
{
	ClearLanes();
}

void VideoChannel::StartCore()
{
	while (!Cancelled())
	{	
		if (_datas.empty())
		{
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else
		{
			long long timeStamp = DateTime::Now().Milliseconds();

			unique_lock<mutex> lck(_dataMutex);
			string data = _datas.front();
			_datas.pop();
			lck.unlock();

			vector<string> vehicles;
			JsonFormatter::Deserialize(data, "Vehicles", &vehicles);
			for (vector<string>::iterator it = vehicles.begin(); it != vehicles.end(); ++it)
			{
				Rectangle detectRegion = GetRegion(*it);
				lock_guard<mutex> lck(_laneMutex);
				for (vector<Lane*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
				{
					(*lit)->PushVehicle(detectRegion, timeStamp, *it);
				}
			}

			vector<string> bikes;
			JsonFormatter::Deserialize(data, "Bikes", &bikes);
			for (vector<string>::iterator it = bikes.begin(); it != bikes.end(); ++it)
			{
				Rectangle detectRegion = GetRegion(*it);
				lock_guard<mutex> lck(_laneMutex);
				for (vector<Lane*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
				{
					(*lit)->PushBike(detectRegion, timeStamp, *it);
				}
			}

			vector<string> pedestrains;
			JsonFormatter::Deserialize(data, "Pedestrains", &pedestrains);
			for (vector<string>::iterator it = pedestrains.begin(); it != pedestrains.end(); ++it)
			{
				Rectangle detectRegion = GetRegion(*it);
				lock_guard<mutex> lck(_laneMutex);
				for (vector<Lane*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
				{
					(*lit)->PushBike(detectRegion, timeStamp, *it);
				}
			}
		}
	}
}

void VideoChannel::UpdateLanes(const vector<Lane*>& lanes)
{
	ClearLanes();
	lock_guard<mutex> lck(_laneMutex);
	_lanes.assign(lanes.begin(), lanes.end());
}

void VideoChannel::Push(const string& data)
{
	lock_guard<mutex> lck(_dataMutex);
	_datas.push(data);
}

string VideoChannel::Collect()
{
	lock_guard<mutex> lck(_laneMutex);
	string json;
	for (vector<Lane*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		LaneItem item = (*it)->Collect();
		JsonFormatter::Serialize(&json, "persons", item.Persons);
		JsonFormatter::Serialize(&json, "bikes", item.Bikes);
		JsonFormatter::Serialize(&json, "motorcycles", item.Motorcycles);
		JsonFormatter::Serialize(&json, "cars", item.Cars);
		JsonFormatter::Serialize(&json, "tricycles", item.Tricycles);
		JsonFormatter::Serialize(&json, "buss", item.Buss);
		JsonFormatter::Serialize(&json, "vans", item.Vans);
		JsonFormatter::Serialize(&json, "trucks", item.Trucks);

		JsonFormatter::Serialize(&json, "averageSpeed", item.Speed);
		JsonFormatter::Serialize(&json, "headDistance", item.HeadDistance);
		JsonFormatter::Serialize(&json, "timeOccupancy", item.TimeOccupancy);

		JsonFormatter::Serialize(&json, "crossingId", (*it)->Id());


	}
	return json;
}

void VideoChannel::ClearLanes()
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<Lane*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
}

Rectangle VideoChannel::GetRegion(const string& data)
{
	string detect;
	JsonFormatter::Deserialize(data, "Detect", &detect);
	string body;
	JsonFormatter::Deserialize(detect, "Body", &body);
	vector<int> datas;
	JsonFormatter::Deserialize(body, "Rect", &datas);
	return Rectangle(datas[0], datas[1], datas[2], datas[3]);
}

