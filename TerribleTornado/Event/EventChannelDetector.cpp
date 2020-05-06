#include "EventChannelDetector.h"

using namespace std;
using namespace OnePunchMan;

const string EventChannelDetector::EventTopic("Event");

EventChannelDetector::EventChannelDetector(int width, int height, MqttChannel* mqtt)
	:ChannelDetector(width,height,mqtt)
{
}

EventChannelDetector::~EventChannelDetector()
{

}

void EventChannelDetector::UpdateChannel(const EventChannel& channel)
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<EventDetector*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
	string regionsParam;
	regionsParam.append("[");

	for (vector<EventLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
	{
		regionsParam.append(lit->PedestrainRegion);
		regionsParam.append(",");
		_lanes.push_back(new EventDetector(*lit));
	}
	if (regionsParam.size() == 1)
	{
		regionsParam.append("]");
	}
	else
	{
		regionsParam[regionsParam.size() - 1] = ']';
	}

	_param = StringEx::Combine("{\"Detect\":{\"DetectRegion\":", regionsParam, ",\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}");
	_setParam = false;
	_channelIndex = channel.ChannelIndex;
	_channelUrl = channel.ChannelUrl;
}

void EventChannelDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<EventDetector*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
	_channelUrl = string();
	_channelIndex = 0;
}


void EventChannelDetector::HandleDetectCore(std::map<std::string, DetectItem> detectItems, long long timeStamp)
{
	string lanesJson;
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
	{
		_lanes[laneIndex]->Detect(&detectItems, timeStamp);
	}
	lck.unlock();

}

