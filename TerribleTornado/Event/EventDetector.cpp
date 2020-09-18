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
const int EventDetector::EncodeIFrameCount = 20;
const int EventDetector::MaxCacheCount = 100;

EventDetector::EventDetector(int width, int height, MqttChannel* mqtt, EncodeChannel* encodeChannel, EventDataChannel* dataChannel)
	:TrafficDetector(width, height, mqtt), _taskId(0), _encodeChannel(encodeChannel), _dataChannel(dataChannel)
{
	_bgrBuffer = new unsigned char[_bgrSize];
	_jpgBuffer = tjAlloc(_jpgSize);
	_videoSize = 10 * 1024 * 1024;
	_videoBuffer = new unsigned char[_videoSize];
}

EventDetector::~EventDetector()
{
	tjFree(_jpgBuffer);
	delete[] _bgrBuffer;
	delete[] _videoBuffer;
}

void EventDetector::UpdateChannel(const unsigned char taskId, const EventChannel& channel)
{
	lock_guard<mutex> lck(_laneMutex);
	_taskId = taskId;
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
					LogPool::Information("init lane ,channel index:", channel.ChannelIndex, "lane index:", lit->LaneIndex, "x offset:", line.Point2.X -line.Point1.X, "y offset:", line.Point2.Y - line.Point1.Y);
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
		LogPool::Warning("not found any lane,channel index:", channel.ChannelIndex);
	}
	_channelIndex = channel.ChannelIndex;
	_param = GetDetectParam(regionsParam);
	_setParam = false;
	_writeBmp = true;
	LogPool::Information(LogEvent::Event, "init event detector,channel index:", channel.ChannelIndex, "task id:", _taskId);
}

void EventDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	_taskId = 0;
	if (_encodeChannel != NULL)
	{
		for (vector<EventData>::iterator it = _encodeDatas.begin(); it != _encodeDatas.end();++it)
		{
			LogPool::Information(LogEvent::Event, "clear event cache,channel index:", _channelIndex, "temp file:", it->GetTempVideo());
			_encodeChannel->RemoveOutput(_channelIndex,it->GetTempVideo());
		}
		_encodeDatas.clear();
	}
	_lanes.clear();
	_channelUrl = string();
	_lanesInited = false;

}

void EventDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, string* param, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex, unsigned char frameSpan)
{
	if (_taskId != taskId)
	{
		return;
	}
	lock_guard<mutex> lck(_laneMutex);
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}
	if (_writeBmp)
	{
		_iveHandler.HandleFrame(iveBuffer, _width, _height, _channelIndex);
		_writeBmp = false;
	}
	if (_encodeChannel != NULL)
	{
		for (vector<EventData>::iterator it = _encodeDatas.begin(); it != _encodeDatas.end();)
		{
			if (_encodeChannel->OutputFinished(_channelIndex,it->GetTempVideo()))
			{
				ImageConvert::IveToJpgFile(iveBuffer, _width, _height, _bgrBuffer, _jpgBuffer, _jpgSize,it->GetTempImage(2));
				_dataChannel->AddData(*it);
				it = _encodeDatas.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		EventLaneCache& laneCache = _lanes[i];
		//删除超时数据
		for (map<string, EventDetectCache>::iterator it = laneCache.Items.begin(); it != laneCache.Items.end();)
		{
			if (timeStamp - it->second.LastTimeStamp > DeleteSpan)
			{
				laneCache.Items.erase(it++);
			}
			else
			{
				++it;
			}
		}

		int carsInLane = 0;
		for (map<string, DetectItem>::iterator dit = detectItems->begin(); dit != detectItems->end(); ++dit)
		{
			if (dit->second.Status != DetectStatus::Out)
			{
				continue;
			}
			if (dit->second.Type == DetectType::Pedestrain
				&& laneCache.LaneType == EventLaneType::Pedestrain
				&& laneCache.Region.Contains(dit->second.Region.HitPoint()))
			{
				map<string, EventDetectCache>::iterator mit = laneCache.Items.find(dit->first);
				if (mit == laneCache.Items.end())
				{
					dit->second.Status = DetectStatus::New;
					EventDetectCache eventItem;
					eventItem.LastTimeStamp = timeStamp;
					laneCache.Items.insert(pair<string, EventDetectCache>(dit->first, eventItem));
					if (_encodeChannel != NULL)
					{
						LogPool::Information(LogEvent::Event, "start pedestrain event encode,channel index:", _channelIndex, "detect id:", dit->first);
						EventData data;
						data.Guid = dit->first;
						data.ChannelIndex = _channelIndex;
						data.ChannelUrl = _channelUrl;
						data.LaneIndex = laneCache.LaneIndex;
						data.TimeStamp = timeStamp;
						data.Type = (int)EventType::Pedestrain;
						if (laneCache.Items.size() < MaxCacheCount)
						{
							if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), EncodeIFrameCount))
							{
								DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
								_encodeDatas.push_back(data);
							}
						}
						else
						{
							LogPool::Warning("event cache queue is full,channel index:", _channelIndex, "lane index:", i + 1, "detect id:", dit->first, "current cache count:", laneCache.Items.size(), "config max cache count:", MaxCacheCount);
						}
					}
					LogPool::Debug(LogEvent::Event, _channelIndex,dit->first, "pedestrain event");
				}
				else
				{
					dit->second.Status = DetectStatus::In;
					mit->second.LastTimeStamp = timeStamp;
				}
			}
			else if (dit->second.Type > DetectType::Motobike
				&& laneCache.Region.Contains(dit->second.Region.HitPoint()))
			{
				if (laneCache.LaneType == EventLaneType::Park)
				{
					map<string, EventDetectCache>::iterator mit = laneCache.Items.find(dit->first);
					if (mit == laneCache.Items.end())
					{
						dit->second.Status = DetectStatus::New;
						EventDetectCache eventItem;
						eventItem.FirstTimeStamp = timeStamp;
						eventItem.LastTimeStamp = timeStamp;
						laneCache.Items.insert(pair<string, EventDetectCache>(dit->first, eventItem));
					}
					else
					{
						dit->second.Status = DetectStatus::In;
						mit->second.LastTimeStamp = timeStamp;
						if (mit->second.StartPark)
						{
							if (!mit->second.StopPark
								&& mit->second.LastTimeStamp - mit->second.FirstTimeStamp > ParkEndSpan)
							{
								mit->second.StopPark = true;
								if (_encodeChannel != NULL)
								{
									LogPool::Information(LogEvent::Event, "start park event encode,channel index:", _channelIndex, "detect id:", dit->first);
									EventData data;
									data.Guid = dit->first;
									data.ChannelIndex = _channelIndex;
									data.ChannelUrl = _channelUrl;
									data.LaneIndex = laneCache.LaneIndex;
									data.TimeStamp = timeStamp;
									data.Type = (int)EventType::Park;
									if (laneCache.Items.size() < MaxCacheCount)
									{
										if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), EncodeIFrameCount))
										{
											DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
											_encodeDatas.push_back(data);
										}
									}
									else
									{
										LogPool::Warning("event cache queue is full,channel index:", _channelIndex, "lane index:", i + 1, "detect id:", dit->first, "current cache count:", laneCache.Items.size(), "config max cache count:", MaxCacheCount);
									}
								}
								LogPool::Debug(LogEvent::Event, _channelIndex, "park event");
							}
						}
						else
						{

							if (mit->second.LastTimeStamp - mit->second.FirstTimeStamp > ParkStartSpan)
							{
								mit->second.StartPark = true;
								ImageConvert::IveToJpgBase64(iveBuffer, _width, _height, _bgrBuffer, _jpgBuffer, _jpgSize, &mit->second.StartParkImage);
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
							if (_encodeChannel != NULL)
							{
								LogPool::Information(LogEvent::Event, "start congestion event encode,channel index:", _channelIndex, "detect id:", dit->first);
								EventData data;
								data.Guid = dit->first;
								data.ChannelIndex = _channelIndex;
								data.ChannelUrl = _channelUrl;
								data.LaneIndex = laneCache.LaneIndex;
								data.TimeStamp = timeStamp;
								data.Type = (int)EventType::Congestion;
								if (laneCache.Items.size() < MaxCacheCount)
								{
									if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), EncodeIFrameCount))
									{
										DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
										_encodeDatas.push_back(data);
									}
								}
								else
								{
									LogPool::Warning("event cache queue is full,channel index:", _channelIndex, "lane index:", i + 1, "detect id:", dit->first, "current cache count:", laneCache.Items.size(), "config max cache count:", MaxCacheCount);
								}
							}
							laneCache.LastReportTimeStamp = timeStamp;
							LogPool::Debug(LogEvent::Event, _channelIndex, "congestion event");
						}
					}
					//处于拥堵状态不检测逆行
					if (!laneCache.Congestion)
					{
						map<string, EventDetectCache>::iterator cit = laneCache.Items.find(dit->first);
						if (cit == laneCache.Items.end())
						{
							dit->second.Status = DetectStatus::New;
							EventDetectCache eventItem;
							eventItem.LastTimeStamp = timeStamp;
							eventItem.RetrogradePoints.push_back(dit->second.Region.HitPoint());
							laneCache.Items.insert(pair<string, EventDetectCache>(dit->first, eventItem));
						}
						else
						{
							dit->second.Status = DetectStatus::In;
							cit->second.LastTimeStamp = timeStamp;
							if (cit->second.RetrogradePoints.size() < PointCount)
							{								
								double distance = dit->second.Region.HitPoint().Distance(cit->second.RetrogradePoints.back());
								if (distance > MovePixel)
								{
									bool xtrend = dit->second.Region.HitPoint().X > cit->second.RetrogradePoints.back().X;
									bool ytrend = dit->second.Region.HitPoint().Y > cit->second.RetrogradePoints.back().Y;
									if ((laneCache.BaseAsX && laneCache.XTrend!= xtrend)
										||(!laneCache.BaseAsX && laneCache.YTrend != ytrend))
									{
										cit->second.RetrogradePoints.push_back(dit->second.Region.HitPoint());
									}
								}
							}
							else if (cit->second.RetrogradePoints.size() == PointCount)
							{
								double distance = dit->second.Region.HitPoint().Distance(cit->second.RetrogradePoints.back());
								if (distance > MovePixel)
								{
									bool xtrend = dit->second.Region.HitPoint().X > cit->second.RetrogradePoints.back().X;
									bool ytrend = dit->second.Region.HitPoint().Y > cit->second.RetrogradePoints.back().Y;
									if ((laneCache.BaseAsX && laneCache.XTrend != xtrend)
										|| (!laneCache.BaseAsX && laneCache.YTrend != ytrend))
									{
										cit->second.RetrogradePoints.push_back(dit->second.Region.HitPoint());
										if (_encodeChannel != NULL)
										{
											LogPool::Information(LogEvent::Event, "start retrograde event encode,channel index:", _channelIndex, "detect id:", dit->first);
											EventData data;
											data.Guid = dit->first;
											data.ChannelIndex = _channelIndex;
											data.ChannelUrl = _channelUrl;
											data.LaneIndex = laneCache.LaneIndex;
											data.TimeStamp = timeStamp;
											data.Type = (int)EventType::Retrograde;
											if (laneCache.Items.size() < MaxCacheCount)
											{
												if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), EncodeIFrameCount))
												{
													DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
													_encodeDatas.push_back(data);
												}
											}
											else
											{
												LogPool::Warning("event cache queue is full,channel index:", _channelIndex, "lane index:", i + 1, "detect id:", dit->first, "current cache count:", laneCache.Items.size(), "config max cache count:", MaxCacheCount);
											}
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
		laneCache.Congestion = carsInLane >= CarCount;
	}
}

void EventDetector::DrawDetect(const string& filePath, const unsigned char* iveBuffer, const Rectangle& detectRegion)
{
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	//绿色检测项区域
	ImageConvert::DrawRectangle(&image, detectRegion, cv::Scalar(0, 255,0));
	ImageConvert::BgrToJpgFile(image.data, _width, _height, _jpgBuffer, _jpgSize,filePath);
}

