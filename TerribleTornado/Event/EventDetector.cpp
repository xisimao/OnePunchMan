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

EventDetector::EventDetector(int width, int height, MqttChannel* mqtt, EncodeChannel* encodeChannel)
	:TrafficDetector(width, height, mqtt), _taskId(0), _encodeChannel(encodeChannel)
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
					LogPool::Information("初始化车道,通道序号:", channel.ChannelIndex, "车道序号:", lit->LaneIndex, "车道基准线x间隔:", line.Point2.X -line.Point1.X, "车道基准线y间隔:", line.Point2.Y - line.Point1.Y);
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
		LogPool::Warning("没有配置车道,通道序号:", channel.ChannelIndex);
	}
	_channelIndex = channel.ChannelIndex;
	_param = GetDetectParam(regionsParam);
	_setParam = false;
	_writeBmp = true;
	LogPool::Information(LogEvent::Event, "初始化事件检测,通道序号:", channel.ChannelIndex, "任务号:", _taskId);
}

void EventDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	_taskId = 0;
	if (_encodeChannel != NULL)
	{
		for (vector<EventEncodeCache>::iterator it = _encodes.begin(); it != _encodes.end();++it)
		{
			LogPool::Information(LogEvent::Event, "清除缓存事件，通道序号：", _channelIndex, "临时文件：", it->FilePath);
			_encodeChannel->RemoveOutput(_channelIndex,it->FilePath);
			remove(it->FilePath.c_str());
		}
		_encodes.clear();
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
		for (vector<EventEncodeCache>::iterator it = _encodes.begin(); it != _encodes.end();)
		{
			if (_encodeChannel->OutputFinished(_channelIndex,it->FilePath))
			{
				string videoBase64;
				ImageConvert::Mp4ToBase64(it->FilePath,_videoBuffer,_videoSize, &videoBase64);
				JsonSerialization::SerializeValue(&it->Json, "video", videoBase64);
				string jpgBase64;
				ImageConvert::IveToJpgBase64(iveBuffer, _width, _height, _bgrBuffer, &jpgBase64, _jpgBuffer, _jpgSize);
				JsonSerialization::SerializeValue(&it->Json, "image2", jpgBase64);
				if (_mqtt != NULL)
				{
					_mqtt->Send(EventTopic, it->Json);
				}
				remove(it->FilePath.c_str());
				it = _encodes.erase(it);
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
						LogPool::Information(LogEvent::Event, "开始行人事件编码，通道序号:", _channelIndex, "检测项编号:", dit->first);
						string filePath = StringEx::Combine("../temp/", dit->first, ".mp4");
						if (laneCache.Items.size() < MaxCacheCount)
						{
							if (_encodeChannel->AddOutput(_channelIndex, filePath, EncodeIFrameCount))
							{
								EventEncodeCache encodeCache;
								JsonSerialization::SerializeValue(&encodeCache.Json, "channelUrl", _channelUrl);
								JsonSerialization::SerializeValue(&encodeCache.Json, "laneIndex", laneCache.LaneIndex);
								JsonSerialization::SerializeValue(&encodeCache.Json, "timeStamp", timeStamp);
								JsonSerialization::SerializeValue(&encodeCache.Json, "type", (int)EventType::Pedestrain);
								string jpgBase64;
								ImageConvert::IveToJpgBase64(iveBuffer, _width, _height, _bgrBuffer, &jpgBase64, _jpgBuffer, _jpgSize);
								JsonSerialization::SerializeValue(&encodeCache.Json, "image1", jpgBase64);
								encodeCache.FilePath = filePath;
								_encodes.push_back(encodeCache);
							}
						}
						else
						{
							LogPool::Warning("事件缓存队列已满，通道序号：", _channelIndex, "车道序号:", i + 1, "检测项编号:", dit->first, "当前缓存数量：", laneCache.Items.size(), "配置的最大缓存数量：", MaxCacheCount);
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
									LogPool::Information(LogEvent::Event, "开始停车事件编码，通道序号:", _channelIndex, "检测项编号:", dit->first);
									string filePath = StringEx::Combine("../temp/", dit->first, ".mp4");
									if (laneCache.Items.size() < MaxCacheCount)
									{
										if (_encodeChannel->AddOutput(_channelIndex, filePath, EncodeIFrameCount))
										{
											EventEncodeCache encodeCache;
											JsonSerialization::SerializeValue(&encodeCache.Json, "channelUrl", _channelUrl);
											JsonSerialization::SerializeValue(&encodeCache.Json, "laneIndex", laneCache.LaneIndex);
											JsonSerialization::SerializeValue(&encodeCache.Json, "timeStamp", timeStamp);
											JsonSerialization::SerializeValue(&encodeCache.Json, "type", (int)EventType::Park);
											string jpgBase64;
											ImageConvert::IveToJpgBase64(iveBuffer, _width, _height, _bgrBuffer, &jpgBase64, _jpgBuffer, _jpgSize);
											JsonSerialization::SerializeValue(&encodeCache.Json, "image1", jpgBase64);
											encodeCache.FilePath = filePath;
											_encodes.push_back(encodeCache);
										}
									}
									else
									{
										LogPool::Warning("事件缓存队列已满，通道序号：", _channelIndex, "车道序号:", i + 1, "检测项编号:", dit->first, "当前缓存数量：", laneCache.Items.size(), "配置的最大缓存数量：", MaxCacheCount);
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
								ImageConvert::IveToJpgBase64(iveBuffer, _width, _height, _bgrBuffer, &mit->second.StartParkImage, _jpgBuffer, _jpgSize);
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
								LogPool::Information(LogEvent::Event, "开始拥堵事件编码，通道序号:", _channelIndex, "检测项编号:", dit->first);
								string filePath = StringEx::Combine("../temp/", dit->first, ".mp4");
								if (laneCache.Items.size() < MaxCacheCount)
								{
									if (_encodeChannel->AddOutput(_channelIndex, filePath, EncodeIFrameCount))
									{
										EventEncodeCache encodeCache;
										JsonSerialization::SerializeValue(&encodeCache.Json, "channelUrl", _channelUrl);
										JsonSerialization::SerializeValue(&encodeCache.Json, "laneIndex", laneCache.LaneIndex);
										JsonSerialization::SerializeValue(&encodeCache.Json, "timeStamp", timeStamp);
										JsonSerialization::SerializeValue(&encodeCache.Json, "type", (int)EventType::Congestion);
										string jpgBase64;
										ImageConvert::IveToJpgBase64(iveBuffer, _width, _height, _bgrBuffer, &jpgBase64, _jpgBuffer, _jpgSize);
										JsonSerialization::SerializeValue(&encodeCache.Json, "image1", jpgBase64);
										encodeCache.FilePath = filePath;
										_encodes.push_back(encodeCache);
									}
								}
								else
								{
									LogPool::Warning("事件缓存队列已满，通道序号：", _channelIndex, "车道序号:", i + 1, "检测项编号:", dit->first, "当前缓存数量：", laneCache.Items.size(), "配置的最大缓存数量：", MaxCacheCount);
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
											LogPool::Information(LogEvent::Event, "开始逆行事件编码，通道序号:", _channelIndex, "检测项编号:", dit->first);
											string filePath= StringEx::Combine("../temp/", dit->first, ".mp4");
											if (laneCache.Items.size() < MaxCacheCount)
											{
												if (_encodeChannel->AddOutput(_channelIndex, filePath, EncodeIFrameCount))
												{
													EventEncodeCache encodeCache;
													JsonSerialization::SerializeValue(&encodeCache.Json, "channelUrl", _channelUrl);
													JsonSerialization::SerializeValue(&encodeCache.Json, "laneIndex", laneCache.LaneIndex);
													JsonSerialization::SerializeValue(&encodeCache.Json, "timeStamp", timeStamp);
													JsonSerialization::SerializeValue(&encodeCache.Json, "type", (int)EventType::Retrograde);
													string jpgBase64;
													ImageConvert::IveToJpgBase64(iveBuffer, _width, _height, _bgrBuffer, &jpgBase64, _jpgBuffer, _jpgSize);
													JsonSerialization::SerializeValue(&encodeCache.Json, "image1", jpgBase64);
													encodeCache.FilePath = filePath;
													_encodes.push_back(encodeCache);
												}
											}
											else
											{
												LogPool::Warning("事件缓存队列已满，通道序号：", _channelIndex, "车道序号:", i + 1, "检测项编号:", dit->first, "当前缓存数量：", laneCache.Items.size(), "配置的最大缓存数量：", MaxCacheCount);
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

void EventDetector::DrawRegion(string* jpgBase64, const unsigned char* iveBuffer, const Polygon& laneRegion, unsigned int frameIndex)
{
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	//红色检测区域
	ImageConvert::DrawPolygon(&image, laneRegion, cv::Scalar(0, 0, 255));
	ImageConvert::BgrToJpgBase64(image.data, _width, _height,jpgBase64, _jpgBuffer, _jpgSize);
}

void EventDetector::DrawRegion(string* jpgBase64, const unsigned char* iveBuffer, const Polygon& laneRegion, const Rectangle& detectRegion, unsigned int frameIndex)
{
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	//红色检测区域
	ImageConvert::DrawPolygon(&image, laneRegion, cv::Scalar(0, 0, 255));
	//绿色检测项区域
	ImageConvert::DrawRectangle(&image, detectRegion, cv::Scalar(0, 255,0));
	ImageConvert::BgrToJpgBase64(image.data, _width, _height,jpgBase64, _jpgBuffer, _jpgSize);
}

