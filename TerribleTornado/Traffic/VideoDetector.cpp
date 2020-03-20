#include "VideoDetector.h"

using namespace std;
using namespace Saitama;

VideoDetector::VideoDetector()
	:Url(),Index()
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

vector<IOItem> VideoDetector::Detect(const std::map<string, DetectItem>& items, long long timeStamp)
{
	vector<IOItem> iOItems;
	lock_guard<mutex> laneLock(_laneMutex);
	for (vector<LaneDetector*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
	{
		IOStatus iOStatus = (*lit)->Detect(items, timeStamp);
		if (iOStatus != IOStatus::UnChanged)
		{
			iOItems.push_back(IOItem(Url, Index, (*lit)->Id(), (*lit)->Index(), (int)iOStatus));
		}
	}
	return iOItems;
}

string VideoDetector::Contains(const DetectItem& item)
{
	lock_guard<mutex> laneLock(_laneMutex);
	for (vector<LaneDetector*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
	{
		if ((*lit)->Contains(item))
		{
			return (*lit)->Id();
		}
	}
	return string();
}


