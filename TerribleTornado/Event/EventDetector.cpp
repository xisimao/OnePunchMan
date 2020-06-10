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
const int EventDetector::MaxEncodeCount = 3;

EventDetector::EventDetector(int width, int height, MqttChannel* mqtt, bool debug)
	:TrafficDetector(width, height, mqtt, debug)
{
	_yuv420pBuffer = new unsigned char[static_cast<int>(width * height * 1.5)];
}

EventDetector::~EventDetector()
{
	delete[] _yuv420pBuffer;
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
	_param = StringEx::Combine("{\"Detect\":{\"DetectRegion\":", regionsParam, ",\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}");
	_setParam = false;
	
}

void EventDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	_lanes.clear();
	_channelUrl = string();
	_lanesInited = false;
}

void EventDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, string* param, const unsigned char* iveBuffer,const unsigned char* yuvBuffer, int frameIndex, int frameSpan)
{
	if (_debug)
	{
		timeStamp = frameIndex * frameSpan;
	}

	if (!_encoders.empty())
	{
		ImageConvert::Yuv420spToYuv420p(yuvBuffer, _width, _height, _yuv420pBuffer);
	}

	for (vector<EventEncoderCache*>::iterator it = _encoders.begin(); it != _encoders.end();)
	{
		EventEncoderCache* cache = *it;
		cache->Encoder.AddYuv(_yuv420pBuffer);
		cache->Image.AddIve(iveBuffer);
		if (cache->Encoder.Finished()&&cache->Image.Finished())
		{
			if (_mqtt != NULL)
			{
				_mqtt->Send(EventTopic, cache->Json);
			}
			it = _encoders.erase(it);
			delete cache;
		}
		else
		{
			++it;
		}
	}

	lock_guard<mutex> lck(_laneMutex);
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& laneCache = _lanes[i];
		//删除超时数据
		map<string, EventDetectCache>::iterator it = laneCache.Items.begin();
		while (it != laneCache.Items.end()) {
			if (timeStamp - it->second.LastTimeStamp > DeleteSpan) {
				laneCache.Items.erase(it++);
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
				&& laneCache.LaneType == EventLaneType::Pedestrain
				&& laneCache.Region.Contains(it->second.Region.HitPoint()))
			{
				map<string, EventDetectCache>::iterator mit = laneCache.Items.find(it->first);
				if (mit == laneCache.Items.end())
				{
					it->second.Status = DetectStatus::New;
					EventDetectCache eventItem;
					eventItem.LastTimeStamp = timeStamp;
					laneCache.Items.insert(pair<string, EventDetectCache>(it->first, eventItem));
					if (_encoders.size() < MaxEncodeCount)
					{
						_encoders.push_back(new EventEncoderCache(_channelUrl, laneCache.LaneIndex, timeStamp, EventType::Pedestrain, StringEx::Combine("../temp/", it->first,".mp4"), _width, _height));
					}
					LogPool::Debug(LogEvent::Event, _channelIndex,it->first, "pedestrain event");
					//DrawPedestrain(iveBuffer, it->second.Region.HitPoint(), frameIndex);
				}
				else
				{
					it->second.Status = DetectStatus::In;
					mit->second.LastTimeStamp = timeStamp;
				}
			}
			else if (it->second.Type > DetectType::Motobike
				&& laneCache.Region.Contains(it->second.Region.HitPoint()))
			{
				if (laneCache.LaneType == EventLaneType::Park)
				{
					map<string, EventDetectCache>::iterator mit = laneCache.Items.find(it->first);
					if (mit == laneCache.Items.end())
					{
						it->second.Status = DetectStatus::New;
						EventDetectCache eventItem;
						eventItem.FirstTimeStamp = timeStamp;
						eventItem.LastTimeStamp = timeStamp;
						laneCache.Items.insert(pair<string, EventDetectCache>(it->first, eventItem));
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
								if (_encoders.size() < MaxEncodeCount)
								{
									_encoders.push_back(new EventEncoderCache(_channelUrl, laneCache.LaneIndex, timeStamp, EventType::Park, StringEx::Combine("../temp/", it->first, ".mp4"), _width, _height));
								}
								LogPool::Debug(LogEvent::Event, _channelIndex, "park event");
								//DrawPark(iveBuffer, it->second.Region.HitPoint(), frameIndex);
							}
						}
						else
						{

							if (mit->second.LastTimeStamp - mit->second.FirstTimeStamp > ParkStartSpan)
							{
								mit->second.StartPark = true;
							}
						}
					}
				}
				else if (laneCache.LaneType == EventLaneType::Lane)
				{
					++carsInLane;
					if (carsInLane == CarCount)
					{
						if (timeStamp - laneCache.LastReportTimeStamp > ReportSpan)
						{
							laneCache.LastReportTimeStamp = timeStamp;
							if (_encoders.size() < MaxEncodeCount)
							{
								_encoders.push_back(new EventEncoderCache(_channelUrl, laneCache.LaneIndex, timeStamp, EventType::Congestion, StringEx::Combine("../temp/", it->first, ".mp4"), _width, _height));
							}
							LogPool::Debug(LogEvent::Event, _channelIndex, "congestion event");		
							//DrawCongestion(iveBuffer, frameIndex);
						}
					}
					//处于拥堵状态不检测逆行
					if (!laneCache.Congestion)
					{
						map<string, EventDetectCache>::iterator mit = laneCache.Items.find(it->first);
						if (mit == laneCache.Items.end())
						{
							it->second.Status = DetectStatus::New;
							EventDetectCache eventItem;
							eventItem.LastTimeStamp = timeStamp;
							eventItem.RetrogradePoints.push_back(it->second.Region.HitPoint());
							laneCache.Items.insert(pair<string, EventDetectCache>(it->first, eventItem));
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
									if ((laneCache.BaseAsX && laneCache.XTrend!= xtrend)
										||(!laneCache.BaseAsX && laneCache.YTrend != ytrend))
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
									if ((laneCache.BaseAsX && laneCache.XTrend != xtrend)
										|| (!laneCache.BaseAsX && laneCache.YTrend != ytrend))
									{
										mit->second.RetrogradePoints.push_back(it->second.Region.HitPoint());
										if (_encoders.size() < MaxEncodeCount)
										{
											_encoders.push_back(new EventEncoderCache(_channelUrl, laneCache.LaneIndex, timeStamp, EventType::Pedestrain, StringEx::Combine("../temp/", it->first, ".mp4"), _width, _height));
										}
										LogPool::Debug(LogEvent::Event, _channelIndex, "retrograde event");
										//DrawRetrograde(iveBuffer, mit->second.RetrogradePoints, frameIndex);
									}
								}
							}
						}
					}
				}
			}
		}
		laneCache.Congestion = carsInLane >= CarCount;
	}
	//DrawDetect(*detectItems, iveBuffer, frameIndex);
}
//
//void EventDetector::DrawPedestrain(const unsigned char* iveBuffer, const Point& point, int frameIndex)
//{
//	if (!_debug)
//	{
//		return;
//	}
//	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
//	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
//	for (unsigned int i = 0; i < _lanes.size(); ++i)
//	{
//		EventLaneCache& cache = _lanes[i];
//		if (cache.LaneType == EventLaneType::Pedestrain)
//		{
//			//红色检测区域
//			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
//		}
//	}
//	//绿色行人点
//	ImageConvert::DrawPoint(&image, point, cv::Scalar(0, 255, 0));
//	int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, &_jpgBuffer, _jpgSize);
//	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, -frameIndex);
//}
//
//void EventDetector::DrawPark(const unsigned char* iveBuffer, const Point& point, int frameIndex)
//{
//	if (!_debug)
//	{
//		return;
//	}
//	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
//	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
//	for (unsigned int i = 0; i < _lanes.size(); ++i)
//	{
//		EventLaneCache& cache = _lanes[i];
//		if (cache.LaneType == EventLaneType::Park)
//		{
//			//红色检测区域
//			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
//		}
//	}
//	//绿色检测车点
//	ImageConvert::DrawPoint(&image, point, cv::Scalar(0, 255, 0));
//	int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, &_jpgBuffer,_jpgSize);
//	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, -frameIndex);
//}
//
//void EventDetector::DrawCongestion(const unsigned char* iveBuffer, int frameIndex)
//{
//	if (!_debug)
//	{
//		return;
//	}
//	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
//	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
//	for (unsigned int i = 0; i < _lanes.size(); ++i)
//	{
//		EventLaneCache& cache = _lanes[i];
//		ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
//	}
//	int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, &_jpgBuffer, _jpgSize);
//	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, -frameIndex);
//}
//
//void EventDetector::DrawRetrograde(const unsigned char* iveBuffer, const vector<Point>& points, int frameIndex)
//{
//	if (!_debug)
//	{
//		return;
//	}
//	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
//	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
//	for (unsigned int i = 0; i < _lanes.size(); ++i)
//	{
//		EventLaneCache& cache = _lanes[i];
//		if (cache.LaneType == EventLaneType::Lane)
//		{
//			//红色检测区域
//			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
//		}
//	}
//
//	//绿色点
//	for (unsigned int i = 0; i < points.size(); ++i)
//	{
//		ImageConvert::DrawPoint(&image, points[i], cv::Scalar(0, 255, 0));
//	}
//
//	int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, &_jpgBuffer, _jpgSize);
//	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, -frameIndex);
//}
//
//void EventDetector::DrawDetect(const map<string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex)
//{
//	if (!_debug)
//	{
//		return;
//	}
//	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
//	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
//	for (unsigned int i = 0; i < _lanes.size(); ++i)
//	{
//		EventLaneCache& cache = _lanes[i];
//		ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
//	}
//	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
//	{
//		cv::Point point(it->second.Region.HitPoint().X, it->second.Region.HitPoint().Y);
//		cv::Scalar scalar;
//		//绿色新车
//		if (it->second.Status == DetectStatus::New)
//		{
//			scalar = cv::Scalar(0, 255, 0);
//		}
//		//黄色在区域
//		else if (it->second.Status == DetectStatus::In)
//		{
//			scalar = cv::Scalar(0, 255, 255);
//		}
//		//蓝色不在区域
//		else
//		{
//			scalar = cv::Scalar(255, 0, 0);
//		}
//		cv::circle(image, point, 10, scalar, -1);
//	}
//
//	int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, &_jpgBuffer, _jpgSize);
//	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, frameIndex);
//}
//
