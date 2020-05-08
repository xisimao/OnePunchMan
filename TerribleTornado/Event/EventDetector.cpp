#include "EventDetector.h"

using namespace std;
using namespace OnePunchMan;

const string EventDetector::EventTopic("Event");

const int EventDetector::DeleteSpan = 60 * 1000;
const int EventDetector::ParkStartSpan = 60 * 1000;
const int EventDetector::ParkEndSpan = 5 * 60 * 1000;
const int EventDetector::CarsCount = 4;
const int EventDetector::ReportSpan = 60*1000;

EventDetector::EventDetector(int width, int height, MqttChannel* mqtt, bool debug)
	:TrafficDetector(width, height, mqtt, debug)
{
}

void EventDetector::UpdateChannel(const EventChannel& channel)
{
	lock_guard<mutex> lck(_laneMutex);
	_lanes.clear();
	string regionsParam;
	regionsParam.append("[");

	for (vector<EventLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
	{
		EventLaneCache cache;
		cache.Region = Polygon::FromJson(lit->Region);
		if (!cache.Region.Empty())
		{
			cache.LaneId = lit->LaneId;
			cache.LaneType = static_cast<EventLaneType>(lit->LaneType);
			if (cache.LaneType == EventLaneType::Lane)
			{
				Line line1 = Line::FromJson(lit->StopLine);
				Line line2 = Line::FromJson(lit->DetectLine);
				if (!line1.Empty()&& !line2.Empty())
				{
					Point point1 = line1.Middle();
					Point point2 = line2.Middle();
					cache.XTrend = point2.X > point1.X;
					cache.YTrend = point2.Y > point1.Y;
					LogPool::Debug("lane", cache.XTrend, cache.YTrend);
					_lanes.push_back(cache);
					regionsParam.append(lit->Region);
					regionsParam.append(",");
				}
			}
			else
			{
				_lanes.push_back(cache);
				regionsParam.append(lit->Region);
				regionsParam.append(",");
			}
		}
	}
	_lanesInited = _lanes.size()==channel.Lanes.size();
	if (!_lanesInited)
	{
		LogPool::Warning("lane init failed", channel.ChannelIndex);
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

void EventDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	_lanes.clear();
	_channelUrl = string();
	_channelIndex = 0;
	_lanesInited = false;
}

void EventDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, string* param, const unsigned char* iveBuffer, long long packetIndex)
{
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}
	string lanesJson;
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		//删除超时数据
		map<string, EventDetectCache>::iterator it = cache.Items.begin();
		while (it != cache.Items.end()) {
			if (timeStamp-it->second.LastTimeStamp > DeleteSpan) {
				cache.Items.erase(it++);
			}
			else {
				++it;
			}
		}
		int carsInLane = 0;
		for (map<string, DetectItem>::iterator it = detectItems->begin(); it != detectItems->end(); ++it)
		{
			if (it->second.Status != DetectStatus::Out)
			{
				continue;
			}
			if (it->second.Type == DetectType::Pedestrain
				&& cache.LaneType== EventLaneType::Pedestrain
				&& cache.Region.Contains(it->second.Region.HitPoint()))
			{
				map<string, EventDetectCache>::iterator mit = cache.Items.find(it->first);
				if (mit == cache.Items.end())
				{
					it->second.Status = DetectStatus::New;
					EventDetectCache eventItem;
					eventItem.LastTimeStamp = timeStamp;
					cache.Items.insert(pair<string, EventDetectCache>(it->first, eventItem));
					string laneJson;
					JsonSerialization::Serialize(&laneJson, "laneId", cache.LaneId);
					JsonSerialization::Serialize(&laneJson, "type", (int)EventType::Pedestrain);
					string jpgBase64;
					GetJpgBase64(&jpgBase64, iveBuffer, packetIndex);
					JsonSerialization::Serialize(&laneJson, "image1", jpgBase64);
					JsonSerialization::SerializeItem(&lanesJson, laneJson);
				}
				else
				{
					it->second.Status = DetectStatus::In;
					mit->second.LastTimeStamp = timeStamp;
				}			
			}
			else if (it->second.Type > DetectType::Motobike
				&& cache.Region.Contains(it->second.Region.HitPoint()))
			{
				if (cache.LaneType == EventLaneType::Park)
				{
					map<string, EventDetectCache>::iterator mit = cache.Items.find(it->first);
					if (mit == cache.Items.end())
					{
						it->second.Status = DetectStatus::New;
						EventDetectCache eventItem;
						eventItem.FirstTimeStamp = timeStamp;
						eventItem.LastTimeStamp = timeStamp;
						cache.Items.insert(pair<string, EventDetectCache>(it->first, eventItem));
					}
					else
					{
						it->second.Status = DetectStatus::In;
						mit->second.LastTimeStamp = timeStamp;
						if (mit->second.StartPark)
						{
							if (!mit->second.StopPark
								&& mit->second.LastTimeStamp - mit->second.FirstTimeStamp > ParkEndSpan)
							{
								mit->second.StopPark = true;
								string laneJson;
								JsonSerialization::Serialize(&laneJson, "laneId", cache.LaneId);
								JsonSerialization::Serialize(&laneJson, "type", (int)EventType::Park);
								JsonSerialization::Serialize(&laneJson, "image1", mit->second.StartParkImage);
								string jpgBase64;
								GetJpgBase64(&jpgBase64, iveBuffer, packetIndex);
								JsonSerialization::Serialize(&laneJson, "image2", jpgBase64);
								JsonSerialization::SerializeItem(&lanesJson, laneJson);
							}
						}
						else
						{

							if (mit->second.LastTimeStamp - mit->second.FirstTimeStamp > ParkStartSpan)
							{
								mit->second.StartPark = true;
								GetJpgBase64(&mit->second.StartParkImage, iveBuffer, packetIndex);
							}
						}
					}
				}
				else if (cache.LaneType == EventLaneType::Lane)
				{
					++carsInLane;
					if (carsInLane == CarsCount)
					{
						if (timeStamp - cache.LastReportTimeStamp > ReportSpan)
						{
							string laneJson;
							JsonSerialization::Serialize(&laneJson, "laneId", cache.LaneId);
							JsonSerialization::Serialize(&laneJson, "type", (int)EventType::Congestion);
							string jpgBase64;
							GetJpgBase64(&jpgBase64, iveBuffer, packetIndex);
							JsonSerialization::Serialize(&laneJson, "image1", jpgBase64);
							JsonSerialization::SerializeItem(&lanesJson, laneJson);
							cache.LastReportTimeStamp = timeStamp;
						}
					}

					map<string, EventDetectCache>::iterator mit = cache.Items.find(it->first);
					if (mit == cache.Items.end())
					{
						it->second.Status = DetectStatus::New;
						EventDetectCache eventItem;
						eventItem.LastTimeStamp = timeStamp;
						eventItem.HitPoint = it->second.Region.HitPoint();
						cache.Items.insert(pair<string, EventDetectCache>(it->first, eventItem));
					}
					else
					{
						it->second.Status = DetectStatus::In;
						mit->second.LastTimeStamp = timeStamp;
						if (!mit->second.StopRetrograde)
						{
							bool xtrend = it->second.Region.HitPoint().X > mit->second.HitPoint.X;
							bool ytrend = it->second.Region.HitPoint().Y > mit->second.HitPoint.Y;
							if (xtrend != cache.XTrend || ytrend != cache.YTrend)
							{
								string laneJson;
								JsonSerialization::Serialize(&laneJson, "laneId", cache.LaneId);
								JsonSerialization::Serialize(&laneJson, "type", (int)EventType::Retrograde);
								string jpgBase64;
								GetJpgBase64(&jpgBase64, iveBuffer, packetIndex);
								JsonSerialization::Serialize(&laneJson, "image1", jpgBase64);
								JsonSerialization::SerializeItem(&lanesJson, laneJson);
								mit->second.StopRetrograde = true;

							}
							mit->second.HitPoint = it->second.Region.HitPoint();
						}
					}
				}
			}	
		}
	}
	lck.unlock();
	if (!lanesJson.empty())
	{
		string channelJson;
		JsonSerialization::SerializeJson(&channelJson, "lanes", lanesJson);
		JsonSerialization::Serialize(&channelJson, "channelUrl", _channelUrl);
		JsonSerialization::Serialize(&channelJson, "timeStamp", timeStamp);

		if (_mqtt != NULL)
		{
			_mqtt->Send(EventTopic, channelJson);
		}
	}

	DrawDetect(*detectItems, iveBuffer, packetIndex);
}

void EventDetector::HandleRecognize(const RecognItem& recognItem, const unsigned char* iveBuffer, const string& recognJson)
{

}

void EventDetector::DrawDetect(const map<string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex)
{
	if (!_debug)
	{
		return;
	}
	IveToBgr(iveBuffer, _width, _height, _debugBgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _debugBgrBuffer);
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		DrawPolygon(&image, cache.Region);
	}
	lck.unlock();

	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		cv::Point point(it->second.Region.HitPoint().X, it->second.Region.HitPoint().Y);
		cv::Scalar scalar;
		//绿色
		if (it->second.Status == DetectStatus::New)
		{
			scalar = cv::Scalar(0, 255, 0);
		}
		//黄色
		else if (it->second.Status == DetectStatus::In)
		{
			scalar = cv::Scalar(0, 255, 255);
		}
		//蓝色
		else
		{
			scalar = cv::Scalar(255, 0, 0);
		}
		cv::circle(image, point, 10, scalar, -1);
	}
	int jpgSize = BgrToJpg(image.data, _width, _height, &_jpgBuffer);
	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, packetIndex);	
}

void EventDetector::GetJpgBase64(std::string* jpgBase64, const unsigned char* iveBuffer, long long packetIndex)
{
	IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	cv::putText(image, DateTime::Now().ToString(), cv::Point(0, 30), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 0), 3);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		DrawPolygon(&image, cache.Region);
	}
	int jpgSize = BgrToJpg(image.data, _width, _height, &_jpgBuffer);
	JpgToBase64(jpgBase64, _jpgBuffer, jpgSize);
	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, packetIndex);
}