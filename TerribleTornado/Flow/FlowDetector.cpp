#include "FlowDetector.h"

using namespace std;
using namespace OnePunchMan;

const string FlowDetector::IOTopic("IO");
const string FlowDetector::FlowTopic("Flow");
const string FlowDetector::VideoStructTopic("VideoStruct");
const int FlowDetector::ReportMaxSpan = 60 * 1000;
const int FlowDetector::DeleteSpan = 60 * 1000;
int FlowDetector::QueueMinDistance = 200;
const int FlowDetector::AllPropertiesFlag = 3;

FlowDetector::FlowDetector(int width, int height, MqttChannel* mqtt, DataMergeMap* merge)
	:TrafficDetector(width, height, mqtt), _merge(merge)
	, _taskId(0)
	, _lastFrameTimeStamp(0), _currentMinuteTimeStamp(0), _nextMinuteTimeStamp(0)
	, _outputImage(false), _outputReport(false), _currentReportMinute(0), _outputRecogn(false)
{
	_detectBgrBuffer = new unsigned char[_bgrSize];
	_detectJpgBuffer = tjAlloc(_jpgSize);

	_recognBgrBuffer = new unsigned char[_bgrSize];
	_recognJpgBuffer = tjAlloc(_jpgSize);
}

FlowDetector::~FlowDetector()
{
	tjFree(_detectJpgBuffer);
	delete[] _detectBgrBuffer;
	tjFree(_recognJpgBuffer);
	delete[] _recognBgrBuffer;
}

void FlowDetector::Init(const JsonDeserialization& jd)
{
	QueueMinDistance = jd.Get<int>("Flow:QueueMinDistance");
	LogPool::Information(LogEvent::Event, "QueueMinDistance", QueueMinDistance, "px");
}

void FlowDetector::UpdateChannel(const unsigned char taskId, const TrafficChannel& channel)
{
	vector<FlowLaneCache> lanes;
	for (vector<FlowLane>::const_iterator it = channel.FlowLanes.begin(); it != channel.FlowLanes.end(); ++it)
	{
		FlowLaneCache laneCache;
		Line stopLine = Line::FromJson(it->StopLine);
		BrokeLine laneLine1 = BrokeLine::FromJson(it->LaneLine1);
		BrokeLine laneLine2 = BrokeLine::FromJson(it->LaneLine2);
		Point point1 = laneLine1.Intersect(stopLine);
		Point point2 = laneLine2.Intersect(stopLine);
		laneCache.FlowRegion = Polygon::FromJson(it->FlowRegion);
		laneCache.QueueRegion = Polygon::FromJson(it->QueueRegion);
		if (point1.Empty()
			|| point2.Empty()
			|| laneCache.FlowRegion.Empty()
			|| laneCache.QueueRegion.Empty())
		{
			LogPool::Warning(LogEvent::Flow, "line empty channel:", it->ChannelIndex, "lane:", it->LaneId);
		}
		else
		{
			double pixels = point1.Distance(point2);
			laneCache.LaneId = it->LaneId;
			laneCache.LaneName = it->LaneName;
			laneCache.Length = it->Length;
			laneCache.Direction = it->Direction;
			laneCache.ReportProperties = it->ReportProperties;
			laneCache.MeterPerPixel = it->Width / pixels;
			laneCache.StopPoint = stopLine.Middle();
			lanes.push_back(laneCache);
		}
	}

	lock_guard<mutex> detectLock(_laneMutex);
	_taskId = taskId;
	_laneCaches.assign(lanes.begin(), lanes.end());
	_channelUrl = channel.ChannelUrl;

	_lanesInited = !_laneCaches.empty() && _laneCaches.size() == channel.FlowLanes.size();
	if (!_lanesInited)
	{
		LogPool::Warning(LogEvent::Flow, "not found any lane,channel index:", channel.ChannelIndex);
	}
	_channelIndex = channel.ChannelIndex;

	//输出图片时先删除旧的
	_outputImage = channel.OutputImage;
	if (_outputImage)
	{
		Command::Execute(StringEx::Combine("rm -rf ", TrafficDirectory::TempDir,"jpg_", channel.ChannelIndex, "*"));
	}
	_reportCaches.clear();
	_vehicleReportCaches.clear();
	_bikeReportCaches.clear();
	_pedestrainReportCaches.clear();
	_outputReport = channel.OutputReport;
	_currentReportMinute = 0;
	DateTime date;
	if (channel.OutputReport)
	{
		date = DateTime::ParseTimeStamp(0);
	}
	else
	{
		date = DateTime::Now();
	}
	_currentMinuteTimeStamp = DateTime(date.Year(), date.Month(), date.Day(), date.Hour(), date.Minute(), 0).TimeStamp();
	_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
	_outputRecogn = channel.OutputRecogn;
	LogPool::Information(LogEvent::Flow, "init flow detector,channel index:", channel.ChannelIndex, " task id:", _taskId, " output report:", channel.OutputReport, " output pic:", channel.OutputImage, " output recogn:", channel.OutputRecogn, " global detect:", channel.GlobalDetect, " current detect timeStamp:", _currentMinuteTimeStamp, " next detect timeStamp:", _nextMinuteTimeStamp);
}

void FlowDetector::ClearChannel()
{
	lock_guard<mutex> detectLock(_laneMutex);
	_taskId = 0;
	_laneCaches.clear();
	_channelUrl = string();
	_lanesInited = false;
}

void FlowDetector::GetReportJson(string* json)
{
	lock_guard<mutex> lock(_laneMutex);
	string flowsJson;
	for (vector<FlowReportData>::iterator it = _reportCaches.begin(); it != _reportCaches.end(); ++it)
	{	
		JsonSerialization::AddClassItem(&flowsJson, it->ToReportJson());
	}
	string vehiclesJson;
	for (vector<VideoStruct_Vehicle>::iterator it = _vehicleReportCaches.begin(); it != _vehicleReportCaches.end(); ++it)
	{
		JsonSerialization::AddClassItem(&vehiclesJson, it->ToJson());
	}
	string bikesJson;
	for (vector<VideoStruct_Bike>::iterator it = _bikeReportCaches.begin(); it != _bikeReportCaches.end(); ++it)
	{
		JsonSerialization::AddClassItem(&bikesJson, it->ToJson());
	}
	string pedestrainsJson;
	for (vector<VideoStruct_Pedestrain>::iterator it = _pedestrainReportCaches.begin(); it != _pedestrainReportCaches.end(); ++it)
	{
		JsonSerialization::AddClassItem(&pedestrainsJson, it->ToJson());
	}
	JsonSerialization::SerializeClass(json, "flows", flowsJson);
	JsonSerialization::SerializeClass(json, "vehicles", vehiclesJson);
	JsonSerialization::SerializeClass(json, "bikes", bikesJson);
	JsonSerialization::SerializeClass(json, "pedestrains", pedestrainsJson);
}

FlowReportData FlowDetector::CalculateMinuteFlow(FlowLaneCache* laneCache)
{
	FlowReportData data;
	data.ChannelUrl = _channelUrl;
	data.LaneId = laneCache->LaneId;
	data.LaneName = laneCache->LaneName;
	data.Direction = laneCache->Direction;
	data.ReportProperties = laneCache->ReportProperties;

	data.Minute = _currentReportMinute;
	data.TimeStamp = _currentMinuteTimeStamp;

	data.Persons = laneCache->Persons;
	data.Bikes = laneCache->Bikes;
	data.Motorcycles = laneCache->Motorcycles;
	data.Cars = laneCache->Cars;
	data.Tricycles = laneCache->Tricycles;
	data.Buss = laneCache->Buss;
	data.Vans = laneCache->Vans;
	data.Trucks = laneCache->Trucks;

	int vehicles = data.Cars + data.Tricycles + data.Buss + data.Vans + data.Trucks;

	//平均速度(km/h)
	data.Speed = laneCache->TotalTime == 0 ? 0 : (laneCache->TotalDistance * laneCache->MeterPerPixel / 1000.0) / (static_cast<double>(laneCache->TotalTime) / 3600000.0);
	//车道时距(sec)
	data.HeadDistance = vehicles > 1 ? static_cast<double>(laneCache->TotalSpan) / static_cast<double>(vehicles - 1) / 1000.0 : 0;
	//车头间距(m)
	data.HeadSpace = data.Speed * 1000 * data.HeadDistance / 3600.0;
	//时间占用率(%)
	data.TimeOccupancy = static_cast<double>(laneCache->TotalInTime) / 60000.0 * 100;
	//交通状态
	if (data.Speed > 40)
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Good);
	}
	else if (data.Speed <= 40 && data.Speed > 30)
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Normal);
	}
	else if (data.Speed <= 30 && data.Speed > 20)
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Warning);
	}
	else if (data.Speed <= 20 && data.Speed > 15)
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Warning);
	}
	else
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Dead);
	}

	data.QueueLength = laneCache->MaxQueueLength;
	//空间占有率(%)
	data.SpaceOccupancy = (laneCache->Length == 0 || laneCache->CountQueueLength == 0) ? 0 : laneCache->TotalQueueLength / static_cast<double>(laneCache->Length) / static_cast<double>(laneCache->CountQueueLength) * 100;

	laneCache->Persons = 0;
	laneCache->Bikes = 0;
	laneCache->Motorcycles = 0;
	laneCache->Tricycles = 0;
	laneCache->Trucks = 0;
	laneCache->Vans = 0;
	laneCache->Cars = 0;
	laneCache->Buss = 0;

	laneCache->TotalDistance = 0.0;
	laneCache->TotalTime = 0;

	laneCache->TotalInTime = 0;

	laneCache->LastInRegion = 0;
	laneCache->TotalSpan = 0;

	laneCache->MaxQueueLength = 0;
	laneCache->TotalQueueLength = 0;
	laneCache->CountQueueLength = 0;
	
	return data;
}

void FlowDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex, unsigned char frameSpan)
{
	if (_taskId != taskId)
	{
		return;
	}
	lock_guard<mutex> detectLock(_laneMutex);
	//输出检测报告时调整时间戳
	if (_outputReport)
	{
		timeStamp = frameIndex * frameSpan;
	}
	//结算上一分钟
	if (timeStamp > _nextMinuteTimeStamp)
	{
		_currentReportMinute += 1;
		string flowLanesJson;
		for (unsigned int i = 0; i < _laneCaches.size(); ++i)
		{
			FlowLaneCache& laneCache = _laneCaches[i];
			FlowReportData data=CalculateMinuteFlow(&laneCache);	
			LogPool::Information(LogEvent::Flow, "original flow->",data.ToString());

			if (laneCache.ReportProperties == AllPropertiesFlag
				||laneCache.ReportProperties==0)
			{			
				JsonSerialization::AddClassItem(&flowLanesJson, data.ToMessageJson());
			}
			else
			{
				_merge->PushData(data);
			}

			if (_outputReport)
			{	
				_reportCaches.push_back(data);
			}
		
		}

		if (!flowLanesJson.empty()
			&& _mqtt != NULL
			&& !_outputReport
			&& (timeStamp - _nextMinuteTimeStamp) < ReportMaxSpan)
		{
		
			_mqtt->Send(FlowTopic, flowLanesJson);
		}

		//结算后认为该分钟结束,当前帧收到的数据结算到下一分钟
		DateTime currentTime=DateTime::ParseTimeStamp(timeStamp);
		DateTime currentTimePoint(currentTime.Year(), currentTime.Month(), currentTime.Day(), currentTime.Hour(), currentTime.Minute(), 0);
		_currentMinuteTimeStamp = currentTimePoint.TimeStamp();
		_lastFrameTimeStamp = _currentMinuteTimeStamp;
		_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
	}

	//计算当前帧
	string ioLanesJson;
	bool hasNewCar = false;
	bool hasQueue = false;
	bool hasIoChanged = false;
	for (unsigned int i = 0; i < _laneCaches.size(); ++i)
	{
		FlowLaneCache& laneCache = _laneCaches[i];
		//删除超时数据
		for (map<string, FlowDetectCache>::iterator it = laneCache.Items.begin(); it != laneCache.Items.end();)
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
		list<double> distances;
		bool ioStatus = false;
		for (map<string, DetectItem>::iterator dit = detectItems->begin(); dit != detectItems->end(); ++dit)
		{
			if (dit->second.Status != DetectStatus::Out)
			{
				continue;
			}
			if (laneCache.FlowRegion.Contains(dit->second.Region.HitPoint()))
			{
				ioStatus = true;
				map<string, FlowDetectCache>::iterator mit = laneCache.Items.find(dit->first);
				//如果是新车则计算流量和车头时距
				//流量=车数量
				//车头时距=所有进入区域的时间差的平均值
				if (mit == laneCache.Items.end())
				{
					dit->second.Status = DetectStatus::New;
					hasNewCar = true;
					if (dit->second.Type == DetectType::Car)
					{
						laneCache.Cars += 1;
					}
					else if (dit->second.Type == DetectType::Tricycle)
					{
						laneCache.Tricycles += 1;
					}
					else if (dit->second.Type == DetectType::Bus)
					{
						laneCache.Buss += 1;
					}
					else if (dit->second.Type == DetectType::Van)
					{
						laneCache.Vans += 1;
					}
					else if (dit->second.Type == DetectType::Truck)
					{
						laneCache.Trucks += 1;
					}
					else if (dit->second.Type == DetectType::Bike)
					{
						laneCache.Bikes += 1;
					}
					else if (dit->second.Type == DetectType::Motobike)
					{
						laneCache.Motorcycles += 1;
					}
					else if (dit->second.Type == DetectType::Pedestrain)
					{
						laneCache.Persons += 1;
					}
					if (laneCache.LastInRegion != 0)
					{
						laneCache.TotalSpan += timeStamp - laneCache.LastInRegion;
					}
					laneCache.LastInRegion = timeStamp;
					FlowDetectCache detectCache;
					detectCache.LastHitPoint = dit->second.Region.HitPoint();
					detectCache.LastTimeStamp = timeStamp;
					laneCache.Items.insert(pair<string, FlowDetectCache>(dit->first, detectCache));
				}
				//如果是已经记录的车则计算平均速度
				//平均速度=总距离/总时间
				//总距离=两次检测到的点的距离*每个像素代表的米数
				//总时间=两次检测到的时间戳时长
				else
				{
					dit->second.Status = DetectStatus::In;
					double distance = dit->second.Region.HitPoint().Distance(mit->second.LastHitPoint);
					laneCache.TotalDistance += distance;
					laneCache.TotalTime += timeStamp - _lastFrameTimeStamp;
					mit->second.LastHitPoint = dit->second.Region.HitPoint();
					mit->second.LastTimeStamp = timeStamp;
				}
			}

			if (laneCache.QueueRegion.Contains(dit->second.Region.HitPoint()))
			{
				dit->second.Distance = dit->second.Region.HitPoint().Distance(laneCache.StopPoint);
				AddOrderedList(&distances, dit->second.Distance);
			}
		}
		//计算排队长度
		laneCache.CurrentQueueLength = CalculateQueueLength(laneCache,distances);
		if (laneCache.CurrentQueueLength != 0)
		{
			hasQueue = true;
			laneCache.TotalQueueLength += laneCache.CurrentQueueLength;
			laneCache.CountQueueLength += 1;
			if (laneCache.CurrentQueueLength > laneCache.MaxQueueLength)
			{
				laneCache.MaxQueueLength = laneCache.CurrentQueueLength;
			}		
		}
	
		//如果上一次有车,则认为到这次检测为止都有车
		//时间占有率=总时间/一分钟
		if (laneCache.IoStatus)
		{
			laneCache.TotalInTime += timeStamp - _lastFrameTimeStamp;
		}

		//io状态改变上报
		if (ioStatus != laneCache.IoStatus)
		{
			hasIoChanged = true;
			laneCache.IoStatus = ioStatus;
			string laneJson;
			JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&laneJson, "laneId", laneCache.LaneId);
			JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
			JsonSerialization::SerializeValue(&laneJson, "status", (int)ioStatus);
			JsonSerialization::AddClassItem(&ioLanesJson, laneJson);
			LogPool::Debug(LogEvent::Flow, "lane:", laneCache.LaneId, "io:", ioStatus);
		}
	}

	//更新时间戳
	_lastFrameTimeStamp = timeStamp;

	//发送数据
	if (!ioLanesJson.empty()
		&& _mqtt != NULL
		&& !_outputReport)
	{
		_mqtt->Send(IOTopic, ioLanesJson);
	}

	//画线
	DrawDetect(*detectItems, hasIoChanged,hasNewCar,hasQueue, iveBuffer, frameIndex);
}

void FlowDetector::FinishDetect(unsigned char taskId)
{
	if (_taskId != taskId)
	{
		return;
	}
	if (_outputReport)
	{
		lock_guard<mutex> lock(_laneMutex);
		_currentReportMinute += 1;
		for (unsigned int i = 0; i < _laneCaches.size(); ++i)
		{
			FlowLaneCache& cache = _laneCaches[i];
			FlowReportData reportCache=CalculateMinuteFlow(&cache);
			_reportCaches.push_back(reportCache);
		}
	}
}

void FlowDetector::HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle)
{
	if (_taskId != recognItem.TaskId)
	{
		return;
	}
	lock_guard<mutex> recognLock(_laneMutex);
	for (vector<FlowLaneCache>::iterator it = _laneCaches.begin(); it != _laneCaches.end(); ++it)
	{
		if (it->Items.find(recognItem.Guid) != it->Items.end())
		{
			string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", it->LaneId);
			JsonSerialization::SerializeValue(&json, "timeStamp", DateTime::NowTimeStamp());
			JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Vehicle);
			JsonSerialization::SerializeValue(&json, "carType", vehicle.CarType);
			JsonSerialization::SerializeValue(&json, "carColor", vehicle.CarColor);
			JsonSerialization::SerializeValue(&json, "carBrand", vehicle.CarBrand);
			JsonSerialization::SerializeValue(&json, "plateType", vehicle.PlateType);
			JsonSerialization::SerializeValue(&json, "plateNumber", vehicle.PlateNumber);
			string image;
			ImageConvert::IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height, _recognBgrBuffer, _recognJpgBuffer, _jpgSize, &image);
			JsonSerialization::SerializeValue(&json, "image", image);
			if (_outputRecogn)
			{
				VideoStruct_Vehicle reportCache = vehicle;
				reportCache.LaneId = it->LaneId;
				reportCache.LaneName = it->LaneName;
				reportCache.Direction = it->Direction;
				reportCache.Minute = recognItem.FrameIndex * recognItem.FrameSpan / 60000;
				reportCache.Second = recognItem.FrameIndex * recognItem.FrameSpan % 60000 / 1000;
				_vehicleReportCaches.push_back(reportCache);
			}
			else
			{
				if (_mqtt != NULL)
				{
					_mqtt->Send(VideoStructTopic, json);
				}
			}
			return;
		}
	}
	LogPool::Debug(LogEvent::Flow, "lost recogn,id:", recognItem.Guid, " type:", static_cast<int>(recognItem.Type), " frame index:", recognItem.FrameIndex);
}

void FlowDetector::HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike)
{
	if (_taskId != recognItem.TaskId)
	{
		return;
	}
	lock_guard<mutex> recognLock(_laneMutex);
	for (vector<FlowLaneCache>::iterator it = _laneCaches.begin(); it != _laneCaches.end(); ++it)
	{
		if (it->Items.find(recognItem.Guid) != it->Items.end())
		{
			string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", it->LaneId);
			JsonSerialization::SerializeValue(&json, "timeStamp", DateTime::NowTimeStamp());
			JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Bike);
			JsonSerialization::SerializeValue(&json, "bikeType", bike.BikeType);
			string image;
			ImageConvert::IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height, _recognBgrBuffer, _recognJpgBuffer, _jpgSize, &image);
			JsonSerialization::SerializeValue(&json, "image", image);
			if (_outputRecogn)
			{
				VideoStruct_Bike reportCache = bike;
				reportCache.LaneId = it->LaneId;
				reportCache.LaneName = it->LaneName;
				reportCache.Direction = it->Direction;
				reportCache.Minute = recognItem.FrameIndex * recognItem.FrameSpan / 60000;
				reportCache.Second = recognItem.FrameIndex * recognItem.FrameSpan %60000/1000;
				_bikeReportCaches.push_back(reportCache);
			}
			else
			{
				if (_mqtt != NULL)
				{
					_mqtt->Send(VideoStructTopic, json);
				}
			}
			return;
		}
	}
	LogPool::Debug(LogEvent::Flow, "lost recogn,id:", recognItem.Guid, " type:", static_cast<int>(recognItem.Type), " frame index:", recognItem.FrameIndex);
}

void FlowDetector::HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain)
{
	if (_taskId != recognItem.TaskId)
	{
		return;
	}
	lock_guard<mutex> recognLock(_laneMutex);
	for (vector<FlowLaneCache>::iterator it = _laneCaches.begin(); it != _laneCaches.end(); ++it)
	{
		if (it->Items.find(recognItem.Guid) != it->Items.end())
		{
			string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", it->LaneId);
			JsonSerialization::SerializeValue(&json, "timeStamp", DateTime::NowTimeStamp());
			JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Pedestrain);
			JsonSerialization::SerializeValue(&json, "sex", pedestrain.Sex);
			JsonSerialization::SerializeValue(&json, "age", pedestrain.Age);
			JsonSerialization::SerializeValue(&json, "upperColor", pedestrain.UpperColor);
			string image;
			ImageConvert::IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height, _recognBgrBuffer, _recognJpgBuffer, _jpgSize, &image);
			JsonSerialization::SerializeValue(&json, "image", image);
			if (_outputRecogn)
			{
				VideoStruct_Pedestrain reportCache = pedestrain;
				reportCache.LaneId = it->LaneId;
				reportCache.LaneName = it->LaneName;
				reportCache.Direction = it->Direction;
				reportCache.Minute = recognItem.FrameIndex * recognItem.FrameSpan / 60000;
				reportCache.Second = recognItem.FrameIndex * recognItem.FrameSpan % 60000 / 1000;
				_pedestrainReportCaches.push_back(reportCache);
			}
			else
			{
				if (_mqtt != NULL)
				{
					_mqtt->Send(VideoStructTopic, json);
				}
			}
			return;
		}
	}
	LogPool::Debug(LogEvent::Flow, "lost recogn,id:", recognItem.Guid, " type:", static_cast<int>(recognItem.Type), " frame index:", recognItem.FrameIndex);
}

void FlowDetector::AddOrderedList(list<double>* distances, double distance)
{
	if (distances->empty())
	{
		distances->push_back(distance);
	}
	else
	{
		for (list<double>::iterator it = distances->begin(); it != distances->end(); ++it)
		{
			if (*it > distance)
			{
				distances->insert(it, distance);
				return;
			}
		}
		distances->push_back(distance);
	}
}

int FlowDetector::CalculateQueueLength(const FlowLaneCache& laneCache, const list<double>& distances)
{
	double maxDistance = 0.0;
	if (distances.size() > 1)
	{
		list<double>::const_iterator preCar = distances.begin();
		list<double>::const_iterator nextCar = ++distances.begin();
		while (preCar != distances.end() && nextCar != distances.end())
		{
			if (*nextCar-*preCar > QueueMinDistance)
			{
				break;
			}
			else
			{
				maxDistance = *nextCar;
			}
			preCar = nextCar;
			++nextCar;
		}
	}
	return static_cast<int>(maxDistance * laneCache.MeterPerPixel);
}

void FlowDetector::DrawDetect(const map<string, DetectItem>& detectItems, bool hasIoChanged, bool hasNewCar, bool hasQueue, const unsigned char* iveBuffer, unsigned int frameIndex)
{
	if (!_outputImage||(!hasIoChanged&&!hasNewCar&&!hasQueue))
	{
		return;
	}

	ImageConvert::IveToBgr(iveBuffer, _width, _height, _detectBgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _detectBgrBuffer);

	//红色标识
	cv::putText(image, string(hasIoChanged ? "V" : "X"), cv::Point(0, 50), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 2);
	cv::putText(image, string(hasNewCar ? "V" : "X"), cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 2);
	cv::putText(image, string(hasQueue ? "V" : "X"), cv::Point(100, 50), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 2);

	//红色区域
	for (unsigned int i = 0; i < _laneCaches.size(); ++i)
	{
		FlowLaneCache& cache = _laneCaches[i];
		ImageConvert::DrawPolygon(&image, cache.FlowRegion, cv::Scalar(0, 0, 255));
		ImageConvert::DrawPolygon(&image, cache.QueueRegion, cv::Scalar(0, 0, 255));
		cv::putText(image, StringEx::ToString(cache.CurrentQueueLength), cv::Point(i * 50, 100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 2);
	}
	//检测项
	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		string detectText;
		if (it->second.Distance > 0)
		{
			detectText.append(StringEx::ToString(static_cast<int>(it->second.Distance)));
		}
		cv::Scalar carScalar;
		cv::Scalar idScalar;
		//绿色新车
		if (it->second.Status == DetectStatus::New)
		{
			carScalar = cv::Scalar(0, 255, 0);

			//设置新车的id颜色
			//红色行人
			if (it->second.Type == DetectType::Pedestrain)
			{
				idScalar = cv::Scalar(0,0,255);
			}
			//黄色非机动车
			else if (it->second.Type == DetectType::Bike || it->second.Type == DetectType::Motobike)
			{
				idScalar = cv::Scalar(255, 255, 0);
			}
			//绿色机动车
			else
			{
				idScalar = cv::Scalar(0, 255, 0);
			}

			//新车输出id文本
			if (!detectText.empty())
			{
				detectText.append("/");
			}
			detectText.append(it->first.substr(it->first.size() - 4, 4));
		}
		//黄色在区域
		else if (it->second.Status == DetectStatus::In)
		{
			carScalar = cv::Scalar(0, 255, 255);
		}
		//蓝色不在区域
		else
		{
			carScalar = cv::Scalar(255, 0, 0);
		}
		if (!detectText.empty())
		{
			ImageConvert::DrawText(&image, detectText, it->second.Region.HitPoint(), idScalar);
		}
		ImageConvert::DrawPoint(&image, it->second.Region.HitPoint(), carScalar);
	}
	int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, _detectJpgBuffer, _jpgSize);
	ImageConvert::JpgToFile(_detectJpgBuffer, jpgSize, _channelIndex, frameIndex);
}
