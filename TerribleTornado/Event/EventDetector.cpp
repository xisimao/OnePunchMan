#include "EventDetector.h"

using namespace std;
using namespace OnePunchMan;

const string EventDetector::EventTopic("Event");

const int EventDetector::DeleteSpan = 60 * 1000;
const int EventDetector::MaxCacheCount = 100;

int EventDetector::ParkStartSpan = 60 * 1000;
int EventDetector::ParkEndSpan = 2 * 60 * 1000;
int EventDetector::CongestionCarCount = 4;
int EventDetector::CongestionReportSpan = 60*1000;
double EventDetector::RetrogradeMinMove = 50.0;
unsigned int EventDetector::RetrogradeMinCount = 3;
int EventDetector::OutputVideoIFrame = 20;


EventDetector::EventDetector(int width, int height, MqttChannel* mqtt, EncodeChannel* encodeChannel, EventDataChannel* dataChannel)
	:TrafficDetector(width, height, mqtt), _taskId(0), _image(width,height,false),_encodeChannel(encodeChannel), _dataChannel(dataChannel)
{
	_videoSize = 10 * 1024 * 1024;
	_videoBuffer = new unsigned char[_videoSize];
}

EventDetector::~EventDetector()
{
	delete[] _videoBuffer;
}

void EventDetector::Init(const JsonDeserialization& jd)
{
	ParkStartSpan = jd.Get<int>("Event:ParkStartSpan");
	ParkEndSpan = jd.Get<int>("Event:ParkEndSpan");
	CongestionCarCount = jd.Get<int>("Event:CongestionCarCount");
	CongestionReportSpan = jd.Get<int>("Event:CongestionReportSpan");
	RetrogradeMinMove = jd.Get<int>("Event:RetrogradeMinMove");
	RetrogradeMinCount = jd.Get<unsigned int>("Event:RetrogradeMinCount");
	OutputVideoIFrame = jd.Get<int>("Event:OutputVideoSpan");
	LogPool::Information(LogEvent::Event, "配置 ParkStartSpan", ParkStartSpan,"sec");
	LogPool::Information(LogEvent::Event, "配置 ParkEndSpan", ParkEndSpan, "sec");
	LogPool::Information(LogEvent::Event, "配置 CongestionCarCount", CongestionCarCount);
	LogPool::Information(LogEvent::Event, "配置 CongestionReportSpan", CongestionReportSpan, "sec");
	LogPool::Information(LogEvent::Event, "配置 RetrogradeMinMove", RetrogradeMinMove, "px");
	LogPool::Information(LogEvent::Event, "配置 RetrogradeMinCount", RetrogradeMinCount);
	LogPool::Information(LogEvent::Event, "配置 OutputVideoSpan", OutputVideoIFrame, "sec");
	ParkStartSpan *= 1000;
	ParkEndSpan *= 1000;
	CongestionReportSpan *= 1000;
	OutputVideoIFrame *= 2;
}

void EventDetector::UpdateChannel(const unsigned char taskId, const TrafficChannel& channel)
{
	lock_guard<mutex> lck(_laneMutex);
	_taskId = taskId;
	_lanes.clear();
	string log(StringEx::Combine("事件检测初始化,通道序号:", channel.ChannelIndex,"任务编号:", _taskId));
	for (vector<EventLane>::const_iterator lit = channel.EventLanes.begin(); lit != channel.EventLanes.end(); ++lit)
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
					log.append(StringEx::Combine("初始化车道,", "车道序号:", lit->LaneIndex, "车道是否根据x轴判断位移:", cache.BaseAsX));	
				}
			}
			else
			{
				_lanes.push_back(cache);
			}
		}
	}
	_channelUrl = channel.ChannelUrl;
	_lanesInited = !_lanes.empty() && _lanes.size() == channel.EventLanes.size();
	if (_lanesInited)
	{
		LogPool::Information(LogEvent::Event, log);
	}
	else
	{
		log.append("没有任何车道配置");
		LogPool::Warning(LogEvent::Event, log);
	}
	_channelIndex = channel.ChannelIndex;
}

void EventDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	_taskId = 0;
	if (_encodeChannel != NULL)
	{
		for (vector<EventData>::iterator it = _encodeDatas.begin(); it != _encodeDatas.end();++it)
		{
			_encodeChannel->RemoveOutput(_channelIndex,it->GetTempVideo());
		}
		_encodeDatas.clear();
	}
	_lanes.clear();
	_channelUrl = string();
	_lanesInited = false;

}

void EventDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex, unsigned char frameSpan)
{
	if (_taskId != taskId)
	{
		return;
	}
	lock_guard<mutex> lck(_laneMutex);
	if (_encodeChannel != NULL)
	{
		for (vector<EventData>::iterator it = _encodeDatas.begin(); it != _encodeDatas.end();)
		{
			if (_encodeChannel->OutputFinished(_channelIndex,it->GetTempVideo()))
			{
				_image.IveToJpgFile(iveBuffer, _width, _height,it->GetTempImage(2));
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
			//机动车
			if (dit->second.Type > DetectType::Motobike)
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
									LogPool::Debug(LogEvent::Event, "开始停车事件检测,通道序号:", _channelIndex, "detect id:", dit->first);
									EventData data;
									data.Guid = dit->first;
									data.ChannelIndex = _channelIndex;
									data.ChannelUrl = _channelUrl;
									data.LaneIndex = laneCache.LaneIndex;
									data.TimeStamp = timeStamp;
									data.Type = (int)EventType::Park;
									if (laneCache.Items.size() < MaxCacheCount)
									{
										if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), OutputVideoIFrame))
										{
											DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
											_encodeDatas.push_back(data);
										}
									}
								}
							}
						}
						else
						{

							if (mit->second.LastTimeStamp - mit->second.FirstTimeStamp > ParkStartSpan)
							{
								mit->second.StartPark = true;
								mit->second.StartParkImage= _image.IveToJpgBase64(iveBuffer, _width, _height);
							
							}
						}
					}
				}
				else if (laneCache.LaneType == EventLaneType::Lane)
				{
					++carsInLane;
					if (carsInLane == CongestionCarCount)
					{
						if (timeStamp - laneCache.LastReportTimeStamp > CongestionReportSpan)
						{
							if (_encodeChannel != NULL)
							{
								LogPool::Debug(LogEvent::Event, "开始拥堵检测,通道序号:", _channelIndex, "detect id:", dit->first);
								EventData data;
								data.Guid = dit->first;
								data.ChannelIndex = _channelIndex;
								data.ChannelUrl = _channelUrl;
								data.LaneIndex = laneCache.LaneIndex;
								data.TimeStamp = timeStamp;
								data.Type = (int)EventType::Congestion;
								if (laneCache.Items.size() < MaxCacheCount)
								{
									if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), OutputVideoIFrame))
									{
										DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
										_encodeDatas.push_back(data);
									}
								}
							}
							laneCache.LastReportTimeStamp = timeStamp;
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
							if (cit->second.RetrogradePoints.size() < RetrogradeMinCount)
							{
								double distance = dit->second.Region.HitPoint().Distance(cit->second.RetrogradePoints.back());
								if (distance > RetrogradeMinMove)
								{
									bool xtrend = dit->second.Region.HitPoint().X > cit->second.RetrogradePoints.back().X;
									bool ytrend = dit->second.Region.HitPoint().Y > cit->second.RetrogradePoints.back().Y;
									if ((laneCache.BaseAsX && laneCache.XTrend != xtrend)
										|| (!laneCache.BaseAsX && laneCache.YTrend != ytrend))
									{
										cit->second.RetrogradePoints.push_back(dit->second.Region.HitPoint());
									}
								}
							}
							else if (cit->second.RetrogradePoints.size() == RetrogradeMinCount)
							{
								double distance = dit->second.Region.HitPoint().Distance(cit->second.RetrogradePoints.back());
								if (distance > RetrogradeMinMove)
								{
									bool xtrend = dit->second.Region.HitPoint().X > cit->second.RetrogradePoints.back().X;
									bool ytrend = dit->second.Region.HitPoint().Y > cit->second.RetrogradePoints.back().Y;
									if ((laneCache.BaseAsX && laneCache.XTrend != xtrend)
										|| (!laneCache.BaseAsX && laneCache.YTrend != ytrend))
									{
										cit->second.RetrogradePoints.push_back(dit->second.Region.HitPoint());
										if (_encodeChannel != NULL)
										{
											LogPool::Debug(LogEvent::Event, "开始逆行检测,通道序号:", _channelIndex, "detect id:", dit->first);
											EventData data;
											data.Guid = dit->first;
											data.ChannelIndex = _channelIndex;
											data.ChannelUrl = _channelUrl;
											data.LaneIndex = laneCache.LaneIndex;
											data.TimeStamp = timeStamp;
											data.Type = (int)EventType::Retrograde;
											if (laneCache.Items.size() < MaxCacheCount)
											{
												if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), OutputVideoIFrame))
												{
													DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
													_encodeDatas.push_back(data);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			//非机动车和行人
			else if(dit->second.Type > DetectType::None || dit->second.Type < DetectType::Car)
			{
				EventType eventType;
				if (laneCache.LaneType == EventLaneType::Pedestrain)
				{
					eventType = EventType::Pedestrain;
				}
				else if (laneCache.LaneType == EventLaneType::Bike)
				{
					eventType = EventType::Bike;
				}
				else
				{
					continue;
				}
				map<string, EventDetectCache>::iterator mit = laneCache.Items.find(dit->first);
				if (mit == laneCache.Items.end())
				{
					dit->second.Status = DetectStatus::New;
					EventDetectCache eventItem;
					eventItem.LastTimeStamp = timeStamp;
					laneCache.Items.insert(pair<string, EventDetectCache>(dit->first, eventItem));
					if (_encodeChannel != NULL)
					{
						LogPool::Debug(LogEvent::Event, "开始",eventType==EventType::Pedestrain?"行人":"非机动车","检测,通道序号:", _channelIndex, "detect id:", dit->first);
						EventData data;
						data.Guid = dit->first;
						data.ChannelIndex = _channelIndex;
						data.ChannelUrl = _channelUrl;
						data.LaneIndex = laneCache.LaneIndex;
						data.TimeStamp = timeStamp;
						data.Type = (int)eventType;
						if (laneCache.Items.size() < MaxCacheCount)
						{
							if (_encodeChannel->AddOutput(_channelIndex, data.GetTempVideo(), OutputVideoIFrame))
							{
								DrawDetect(data.GetTempImage(1), iveBuffer, dit->second.Region);
								_encodeDatas.push_back(data);
							}
						}
					}
				}
				else
				{
					dit->second.Status = DetectStatus::In;
					mit->second.LastTimeStamp = timeStamp;
				}
			}		
		}
		laneCache.Congestion = carsInLane >= CongestionCarCount;
	}
}

void EventDetector::DrawDetect(const string& filePath, const unsigned char* iveBuffer, const Rectangle& detectRegion)
{
	_image.IveToBgr(iveBuffer, _width, _height);
	cv::Mat image(_height, _width, CV_8UC3, _image.GetBgrBuffer());
	//绿色检测项区域
	ImageDrawing::DrawRectangle(&image, detectRegion, cv::Scalar(0, 255,0));
	_image.BgrToJpgFile(image.data, _width, _height,filePath);
}

