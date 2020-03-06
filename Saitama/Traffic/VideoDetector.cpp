#include "VideoDetector.h"

using namespace std;
using namespace Saitama;

VideoDetector::VideoDetector()
	:Id(),Index()
{

}

VideoDetector::~VideoDetector()
{
	ClearLanes();
}

void VideoDetector::ClearLanes()
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<LaneDetector*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
}


void VideoDetector::UpdateLanes(const vector<LaneDetector*>& lanes)
{
	ClearLanes();
	lock_guard<mutex> lck(_laneMutex);
	_lanes.assign(lanes.begin(), lanes.end());
}

vector<IOItem> VideoDetector::Detect(const std::vector<DetectItem>& vehicles, const std::vector<DetectItem>& bikes, std::vector<DetectItem>& pedestrains)
{
	vector<IOItem> items;
	lock_guard<mutex> laneLock(_laneMutex);
	for (vector<LaneDetector*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
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

vector<LaneItem> VideoDetector::Collect()
{
	vector<LaneItem> lanes;
	lock_guard<mutex> lck(_laneMutex);
	for (vector<LaneDetector*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		lanes.push_back((*it)->Collect());
	}
	return lanes;
}


