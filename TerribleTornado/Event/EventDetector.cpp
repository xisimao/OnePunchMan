#include "EventDetector.h"

using namespace std;
using namespace OnePunchMan;

const string EventDetector::EventTopic("Event");

const int EventDetector::DeleteSpan = 60 * 1000;
const int EventDetector::ParkStartSpan = 60 * 1000;
const int EventDetector::ParkEndSpan = 5 * 60 * 1000;
const int EventDetector::CarCount = 4;
const int EventDetector::ReportSpan = 60*1000;
const double EventDetector::MovePixel = 50.0;
const int EventDetector::PointCount = 3;

EventDetector::EventDetector(int width, int height, MqttChannel* mqtt)
	:TrafficDetector(width, height, mqtt)
{
	_bgrBuffer = new unsigned char[_bgrSize];
	_jpgBuffer = tjAlloc(_jpgSize);
}

EventDetector::~EventDetector()
{
	tjFree(_jpgBuffer);
	delete[] _bgrBuffer;
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
			cache.LaneIndex = lit->LaneIndex;
			cache.LaneType = static_cast<EventLaneType>(lit->LaneType);
			if (cache.LaneType == EventLaneType::Lane)
			{
				Line line = Line::FromJson(lit->Line);
				if (!line.Empty())
				{
					cache.XTrend = line.Point2.X > line.Point1.X;
					cache.YTrend = line.Point2.Y > line.Point1.Y;
					cache.BaseAsX = abs(line.Point2.X - line.Point1.X) > abs(line.Point2.Y - line.Point1.Y);
					_lanes.push_back(cache);
					LogPool::Information("init lane", channel.ChannelIndex, lit->LaneIndex, line.Point2.X -line.Point1.X, line.Point2.Y - line.Point1.Y);
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
	if (regionsParam.size() == 1)
	{
		regionsParam.append("]");
	}
	else
	{
		regionsParam[regionsParam.size() - 1] = ']';
	}
	_channelUrl = channel.ChannelUrl;

	_lanesInited = !_lanes.empty() && _lanes.size() == channel.Lanes.size();
	if (!_lanesInited)
	{
		LogPool::Warning("lane init failed", channel.ChannelIndex);
	}
	_channelIndex = channel.ChannelIndex;
	_param = GetDetectParam(regionsParam);
	_setParam = false;
	
}

void EventDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	_lanes.clear();
	_channelUrl = string();
	_lanesInited = false;
}

void EventDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, string* param, const unsigned char* iveBuffer, int frameIndex, int frameSpan)
{
	lock_guard<mutex> lck(_laneMutex);
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		//删除超时数据
		map<string, EventDetectCache>::iterator it = cache.Items.begin();
		while (it != cache.Items.end()) {
			if (timeStamp - it->second.LastTimeStamp > DeleteSpan) {
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
				&& cache.LaneType == EventLaneType::Pedestrain
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
					JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
					JsonSerialization::SerializeValue(&laneJson, "laneIndex", cache.LaneIndex);
					JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
					JsonSerialization::SerializeValue(&laneJson, "type", (int)EventType::Pedestrain);
					string jpgBase64;
					DrawPedestrain(&jpgBase64, iveBuffer, it->second.Region.HitPoint(), frameIndex);
					JsonSerialization::SerializeValue(&laneJson, "image1", jpgBase64);
					if (_mqtt != NULL)
					{
						_mqtt->Send(EventTopic, laneJson);
					}
					LogPool::Debug(LogEvent::Event, _channelIndex,it->first, "pedestrain event");
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
								JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
								JsonSerialization::SerializeValue(&laneJson, "laneIndex", cache.LaneIndex);
								JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
								JsonSerialization::SerializeValue(&laneJson, "type", (int)EventType::Park);
								JsonSerialization::SerializeValue(&laneJson, "image1", mit->second.StartParkImage);
								string jpgBase64;
								DrawPark(&jpgBase64, iveBuffer, it->second.Region.HitPoint(), frameIndex);
								JsonSerialization::SerializeValue(&laneJson, "image2", jpgBase64);
								if (_mqtt != NULL)
								{
									_mqtt->Send(EventTopic, laneJson);
								}
								LogPool::Debug(LogEvent::Event, _channelIndex, "park event");
							}
						}
						else
						{

							if (mit->second.LastTimeStamp - mit->second.FirstTimeStamp > ParkStartSpan)
							{
								mit->second.StartPark = true;
								DrawPark(&mit->second.StartParkImage, iveBuffer, it->second.Region.HitPoint(), frameIndex);
							}
						}
					}
				}
				else if (cache.LaneType == EventLaneType::Lane)
				{
					++carsInLane;
					if (carsInLane == CarCount)
					{
						if (timeStamp - cache.LastReportTimeStamp > ReportSpan)
						{
							string laneJson;
							JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
							JsonSerialization::SerializeValue(&laneJson, "laneIndex", cache.LaneIndex);
							JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
							JsonSerialization::SerializeValue(&laneJson, "type", (int)EventType::Congestion);
							string jpgBase64;
							DrawCongestion(&jpgBase64, iveBuffer, frameIndex);
							JsonSerialization::SerializeValue(&laneJson, "image1", jpgBase64);
							if (_mqtt != NULL)
							{
								_mqtt->Send(EventTopic, laneJson);
							}
							cache.LastReportTimeStamp = timeStamp;
							LogPool::Debug(LogEvent::Event, _channelIndex, "congestion event");
						}
					}
					//处于拥堵状态不检测逆行
					if (!cache.Congestion)
					{
						map<string, EventDetectCache>::iterator mit = cache.Items.find(it->first);
						if (mit == cache.Items.end())
						{
							it->second.Status = DetectStatus::New;
							EventDetectCache eventItem;
							eventItem.LastTimeStamp = timeStamp;
							eventItem.RetrogradePoints.push_back(it->second.Region.HitPoint());
							cache.Items.insert(pair<string, EventDetectCache>(it->first, eventItem));
						}
						else
						{
							it->second.Status = DetectStatus::In;
							mit->second.LastTimeStamp = timeStamp;
							if (mit->second.RetrogradePoints.size() < PointCount)
							{								
								double distance = it->second.Region.HitPoint().Distance(mit->second.RetrogradePoints.back());
								if (distance > MovePixel)
								{
									bool xtrend = it->second.Region.HitPoint().X > mit->second.RetrogradePoints.back().X;
									bool ytrend = it->second.Region.HitPoint().Y > mit->second.RetrogradePoints.back().Y;
									if ((cache.BaseAsX && cache.XTrend!= xtrend)
										||(!cache.BaseAsX && cache.YTrend != ytrend))
									{
										mit->second.RetrogradePoints.push_back(it->second.Region.HitPoint());
									}
								}
							}
							else if (mit->second.RetrogradePoints.size() == PointCount)
							{
								double distance = it->second.Region.HitPoint().Distance(mit->second.RetrogradePoints.back());
								if (distance > MovePixel)
								{
									bool xtrend = it->second.Region.HitPoint().X > mit->second.RetrogradePoints.back().X;
									bool ytrend = it->second.Region.HitPoint().Y > mit->second.RetrogradePoints.back().Y;
									if ((cache.BaseAsX && cache.XTrend != xtrend)
										|| (!cache.BaseAsX && cache.YTrend != ytrend))
									{
										mit->second.RetrogradePoints.push_back(it->second.Region.HitPoint());
										string laneJson;
										JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
										JsonSerialization::SerializeValue(&laneJson, "laneIndex", cache.LaneIndex);
										JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
										JsonSerialization::SerializeValue(&laneJson, "type", (int)EventType::Retrograde);
										string jpgBase64;
										DrawRetrograde(&jpgBase64, iveBuffer, mit->second.RetrogradePoints, frameIndex);
										JsonSerialization::SerializeValue(&laneJson, "image1", jpgBase64);
										if (_mqtt != NULL)
										{
											_mqtt->Send(EventTopic, laneJson);
										}
										LogPool::Debug(LogEvent::Event, _channelIndex, "retrograde event");
									}
								}
							}
						}
					}
				}
			}
		}
		cache.Congestion = carsInLane >= CarCount;
	}


	DrawDetect(*detectItems, iveBuffer, frameIndex);
}

void EventDetector::DrawPedestrain(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, int frameIndex)
{
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		if (cache.LaneType == EventLaneType::Pedestrain)
		{
			//红色检测区域
			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
		}
	}
	//绿色行人点
	ImageConvert::DrawPoint(&image, point, cv::Scalar(0, 255, 0));
	ImageConvert::BgrToJpgBase64(image.data, _width, _height,jpgBase64, _jpgBuffer, _jpgSize);
}

void EventDetector::DrawPark(std::string* jpgBase64, const unsigned char* iveBuffer, const Point& point, int frameIndex)
{
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		if (cache.LaneType == EventLaneType::Park)
		{
			//红色检测区域
			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
		}
	}
	//绿色检测车点
	ImageConvert::DrawPoint(&image, point, cv::Scalar(0, 255, 0));
	ImageConvert::BgrToJpgBase64(image.data, _width, _height,jpgBase64, _jpgBuffer,_jpgSize);
}

void EventDetector::DrawCongestion(string* jpgBase64, const unsigned char* iveBuffer, int frameIndex)
{
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	//cv::putText(image, DateTime::Now().ToString(), cv::Point(0, 30), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 0), 3);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
	}
	ImageConvert::BgrToJpgBase64(image.data, _width, _height,jpgBase64, _jpgBuffer, _jpgSize);
}

void EventDetector::DrawRetrograde(string* jpgBase64, const unsigned char* iveBuffer, const vector<Point>& points, int frameIndex)
{
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& cache = _lanes[i];
		if (cache.LaneType == EventLaneType::Lane)
		{
			//红色检测区域
			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
		}
	}

	//绿色点
	for (unsigned int i = 0; i < points.size(); ++i)
	{
		ImageConvert::DrawPoint(&image, points[i], cv::Scalar(0, 255, 0));
	}

	ImageConvert::BgrToJpgBase64(image.data, _width, _height,jpgBase64, _jpgBuffer, _jpgSize);
}

void EventDetector::DrawDetect(const map<string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex)
{
	//if (!_outputImage)
	//{
	//	return;
	//}
	//ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	//cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	//for (unsigned int i = 0; i < _lanes.size(); ++i)
	//{
	//	EventLaneCache& cache = _lanes[i];
	//	ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
	//}
	//for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	//{
	//	cv::Point point(it->second.Region.HitPoint().X, it->second.Region.HitPoint().Y);
	//	cv::Scalar scalar;
	//	//绿色新车
	//	if (it->second.Status == DetectStatus::New)
	//	{
	//		scalar = cv::Scalar(0, 255, 0);
	//	}
	//	//黄色在区域
	//	else if (it->second.Status == DetectStatus::In)
	//	{
	//		scalar = cv::Scalar(0, 255, 255);
	//	}
	//	//蓝色不在区域
	//	else
	//	{
	//		scalar = cv::Scalar(255, 0, 0);
	//	}
	//	cv::circle(image, point, 10, scalar, -1);
	//}

	//int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, _jpgBuffer, _jpgSize);
	//if (_outputImage)
	//{
	//	ImageConvert::JpgToFile(_jpgBuffer, jpgSize, _channelIndex, frameIndex);
	//}
}

