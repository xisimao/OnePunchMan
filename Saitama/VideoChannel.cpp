#include "VideoChannel.h"

using namespace std;
using namespace Saitama;

void VideoChannel::StartCore()
{
	while (!Cancelled())
	{	
		if (_messages.empty())
		{
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else
		{
			std::unique_lock<std::mutex> lck(_mutex);
			string message = _messages.front();
			_messages.pop();
			lck.unlock();

			long long timeStamp = DateTime::Now().Milliseconds();
			vector<string> vehicles = JsonFormatter::GetArray(message, "Vehicles");
			for (std::vector<string>::iterator it = vehicles.begin(); it != vehicles.end(); ++it)
			{
				string detectValue = JsonFormatter::GetClass(*it, "Detect");
				string bodyValue = JsonFormatter::GetClass(detectValue, "Body");
				vector<int> datas;
				JsonFormatter::Deserialize(bodyValue, tuple<string, vector<int>*>("Rect", &datas));
				Rectangle detectRegion(datas[0], datas[1], datas[2], datas[3]);

				for (std::vector<Lane>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
				{
					lit->CollectVehicle(detectRegion, timeStamp, *it);
				}
			}

			vector<string> bikes = JsonFormatter::GetArray(message, "Bikes");
			for (std::vector<string>::iterator it = bikes.begin(); it != bikes.end(); ++it)
			{
				string detectValue = JsonFormatter::GetClass(*it, "Detect");
				string bodyValue = JsonFormatter::GetClass(detectValue, "Body");
				vector<int> datas;
				JsonFormatter::Deserialize(bodyValue, tuple<string, vector<int>*>("Rect", &datas));
				Rectangle detectRegion(datas[0], datas[1], datas[2], datas[3]);
				for (std::vector<Lane>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
				{
					lit->CollectBike(detectRegion, timeStamp, *it);
				}
			}

			vector<string> pedestrains = JsonFormatter::GetArray(message, "Pedestrains");
			for (std::vector<string>::iterator it = pedestrains.begin(); it != pedestrains.end(); ++it)
			{
				string detectValue = JsonFormatter::GetClass(*it, "Detect");
				string bodyValue = JsonFormatter::GetClass(detectValue, "Body");
				vector<int> datas;
				JsonFormatter::Deserialize(bodyValue, tuple<string, vector<int>*>("Rect", &datas));
				Rectangle detectRegion(datas[0], datas[1], datas[2], datas[3]);
				for (std::vector<Lane>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
				{
					lit->CollectBike(detectRegion, timeStamp, *it);
				}
			}
		}
	}
}

void VideoChannel::Push(const std::string& message)
{
	std::lock_guard<std::mutex> lck(_mutex);
	_messages.push(message);
}