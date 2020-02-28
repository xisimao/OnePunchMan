#include "Channel.h"

using namespace std;
using namespace Saitama;

Channel::Channel()
	:Id(),Index()
{

}

Channel::~Channel()
{
	ClearLanes();
}

void Channel::ClearLanes()
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<Lane*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
}


void Channel::UpdateLanes(const vector<Lane*>& lanes)
{
	ClearLanes();
	lock_guard<mutex> lck(_laneMutex);
	_lanes.assign(lanes.begin(), lanes.end());
}

vector<IOItem> Channel::Detect(const std::vector<DetectItem>& vehicles, const std::vector<DetectItem>& bikes, std::vector<DetectItem>& pedestrains)
{
	vector<IOItem> items;
	lock_guard<mutex> laneLock(_laneMutex);
	for (vector<Lane*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
	{
		bool status = false;
		for (vector<DetectItem>::const_iterator dit = vehicles.begin(); dit != vehicles.end(); ++dit)
		{
			if ((*lit)->DetectVehicle(*dit))
			{
				status = true;
			}
		}

		for (vector<DetectItem>::const_iterator bit = bikes.begin(); bit != bikes.end(); ++bit)
		{
			if ((*lit)->DetectBike(*bit))
			{
				status = true;
			}
		}

		for (vector<DetectItem>::const_iterator pit = pedestrains.begin(); pit != pedestrains.end(); ++pit)
		{
			if ((*lit)->DetectPedestrain(*pit))
			{
				status = true;
			}
		}

		if ((*lit)->Status != status)
		{
			items.push_back(IOItem(Id, Index, (*lit)->Id(), (*lit)->Index(), status?1:0));
			(*lit)->Status = status;
		}
	}
	return items;
}

vector<LaneItem> Channel::Collect()
{
	vector<LaneItem> lanes;
	lock_guard<mutex> lck(_laneMutex);
	for (vector<Lane*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		lanes.push_back((*it)->Collect());
	}
	return lanes;
}


