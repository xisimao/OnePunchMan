#include "EventDetector.h"

using namespace std;
using namespace OnePunchMan;

EventDetector::EventDetector(const EventLane& lane)
	:_laneId(lane.LaneId)
{
	_pedestrainRegion = Polygon::FromJson(lane.PedestrainRegion);
	if (_pedestrainRegion.Empty())
	{
	
	}
	_parkRegions = Polygon::FromJsonArray(lane.ParkRegions);

}

vector<EventResult> EventDetector::Detect(std::map<std::string, DetectItem>* items, long long timeStamp)
{
	lock_guard<mutex> lck(_mutex);
	_currentItems->clear();

	vector<EventResult> results;
	for (map<string, DetectItem>::iterator it = items->begin(); it != items->end(); ++it)
	{
		if (it->second.Status != DetectStatus::Out)
		{
			continue;
		}
		if (it->second.Type == DetectType::Pedestrain
			&& _pedestrainRegion.Contains(it->second.Region.HitPoint()))
		{
			map<string, DetectItem>::const_iterator mit = _lastItems->find(it->first);
			if (mit == _lastItems->end())
			{
				it->second.Status = DetectStatus::New;
				EventResult result;
				result.LaneId = _laneId;
				result.Type = EventType::Pedestrain;
				results.push_back(result);
			}
			else
			{
				it->second.Status = DetectStatus::In;
			}
			_currentItems->insert(pair<string, DetectItem>(it->first, it->second));
		}		
	}

	map<string, DetectItem>* temp = _currentItems;
	_currentItems = _lastItems;
	_lastItems = temp;
	return results;
}
