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
	, _detectImage(width,height,false),_recognImage(width,height,false)
	, _outputReport(false), _currentReportMinute(0), _outputRecogn(false)
{

}

void FlowDetector::Init(const JsonDeserialization& jd)
{
	QueueMinDistance = jd.Get<int>("Flow:QueueMinDistance");
	LogPool::Information(LogEvent::Event, "QueueMinDistance", QueueMinDistance, "px");
}

void FlowDetector::UpdateChannel(const unsigned char taskId, const TrafficChannel& channel)
{
	string log(StringEx::Combine("初始化流量检测,通道序号:", channel.ChannelIndex, "任务编号:", _taskId));

	vector<FlowLaneCache> lanes;
	for (vector<FlowLane>::const_iterator it = channel.FlowLanes.begin(); it != channel.FlowLanes.end(); ++it)
	{
		FlowLaneCache laneCache;
		Line stopLine = Line::FromJson(it->StopLine);
		Line queueDetectLine = Line::FromJson(it->QueueDetectLine);
		BrokenLine laneLine1 = BrokenLine::FromJson(it->LaneLine1);
		BrokenLine laneLine2 = BrokenLine::FromJson(it->LaneLine2);
		laneCache.FlowRegion = Polygon::FromJson(it->FlowRegion);
		laneCache.QueueRegion = Polygon::FromJson(it->QueueRegion);
		if (stopLine.Empty()
			|| queueDetectLine.Empty()
			|| laneCache.FlowRegion.Empty()
			|| laneCache.QueueRegion.Empty())
		{
			log.append(StringEx::Combine("通道:",it->ChannelIndex,"车道:",it->LaneIndex,"初始化失败"));
		}
		else
		{
			double pixels = (laneLine1.Points().front().Distance(laneLine2.Points().front())+ laneLine1.Points().back().Distance(laneLine2.Points().back()))/2;
			laneCache.LaneIndex = it->LaneIndex;
			laneCache.LaneId = it->LaneId;
			laneCache.LaneName = it->LaneName;
			laneCache.Direction = it->Direction;
			laneCache.ReportProperties = it->ReportProperties;
			laneCache.MeterPerPixel = channel.LaneWidth / pixels;
			laneCache.Length = stopLine.Middle().Distance(queueDetectLine.Middle())*laneCache.MeterPerPixel;
			laneCache.StopPoint = stopLine.Middle();
			lanes.push_back(laneCache);
		
			log.append(StringEx::Combine("通道:", it->ChannelIndex, "车道:", it->LaneIndex, "初始化成功","流量区域:",laneCache.FlowRegion.ToJson(),"排队区域:",laneCache.QueueRegion.ToJson(),"换算比例:", laneCache.MeterPerPixel,"m/px","排队区域长度:", laneCache.Length));
		}
	}

	lock_guard<mutex> detectLock(_laneMutex);
	_taskId = taskId;
	_laneCaches.assign(lanes.begin(), lanes.end());
	_channelUrl = channel.ChannelUrl;

	_lanesInited = !_laneCaches.empty() && _laneCaches.size() == channel.FlowLanes.size();
	_channelIndex = channel.ChannelIndex;

	_reportCaches.clear();
	_vehicleReportCaches.clear();
	_bikeReportCaches.clear();
	_pedestrainReportCaches.clear();
	_outputReport = channel.OutputReport;
	_outputRecogn = channel.OutputRecogn;
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
	log.append(StringEx::Combine("时间段:", DateTime::ParseTimeStamp(_currentMinuteTimeStamp).ToString(), "->", DateTime::ParseTimeStamp(_nextMinuteTimeStamp).ToString()));
	if (_lanesInited)
	{
		LogPool::Information(LogEvent::Flow, log);	
	}
	else
	{
		log.append("没有任何车道配置");
		LogPool::Warning(LogEvent::Flow, log);
	}
}

void FlowDetector::ClearChannel()
{
	lock_guard<mutex> detectLock(_laneMutex);
	_taskId = 0;
	_laneCaches.clear();
	_channelUrl = string();
	_lanesInited = false;
}

void FlowDetector::ResetTimeRange()
{
	_currentMinuteTimeStamp = 0;
	_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
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
	//车头时距(sec)
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
	//排队长度(m)
	data.QueueLength = laneCache->MaxQueueLength;
	//空间占有率(%)
	data.SpaceOccupancy = (laneCache->Length == 0 || laneCache->CountQueueLength == 0) ? 0 : laneCache->TotalQueueLength / static_cast<double>(laneCache->Length) / static_cast<double>(laneCache->CountQueueLength) * 100;

	LogPool::Information(LogEvent::Flow,"流量数据 通道序号:", _channelIndex, "车道序号:", laneCache->LaneIndex,"当前时间:",DateTime::ParseTimeStamp(_currentMinuteTimeStamp).ToString()
		, "轿车:", data.Cars, "卡车:", data.Trucks, "客车:", data.Buss, "面包车:", data.Vans, "三轮车:", data.Tricycles, "摩托车:", data.Motorcycles, "自行车:", data.Bikes, "行人:", data.Persons
		, "平均速度(km/h):", data.Speed, "总移动像素(px):", laneCache->TotalDistance, "每像素代表距离(m/px):", laneCache->MeterPerPixel, "总行车时间(ms)", laneCache->TotalTime
		, "车头时距(sec):", data.HeadDistance, "车辆进入区域时间差的和(ms):", laneCache->TotalSpan, "机动车总数:", vehicles
		, "车头间距(m):", data.HeadSpace
		, "时间占有率(%):", data.TimeOccupancy, "区域占用总时间(ms):", laneCache->TotalInTime
		, "排队长度(m):", data.QueueLength
		, "空间占有率(%):", data.SpaceOccupancy, "总排队长度(m):", laneCache->TotalQueueLength, "车道长度(m):", laneCache->Length, "总排队长度计数次数:", laneCache->CountQueueLength);

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
	laneCache->CurrentQueueLength = 0;

	return data;
}

void FlowDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan)
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
			laneCache.IoStatus = ioStatus;
			string laneJson;
			JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&laneJson, "laneId", laneCache.LaneId);
			JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
			JsonSerialization::SerializeValue(&laneJson, "status", (int)ioStatus);
			JsonSerialization::AddClassItem(&ioLanesJson, laneJson);
			LogPool::Information(LogEvent::Flow, "IO数据 通道序号:", _channelIndex, "车道序号:", laneCache.LaneIndex, "当前时间:", DateTime::ParseTimeStamp(timeStamp).ToString(), "IO状态:", ioStatus);
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
			string image = _recognImage.IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height);
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
			string image = _recognImage.IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height);
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
			string image=_recognImage.IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height);
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

void FlowDetector::DrawDetect(const map<string, DetectItem>& detectItems, unsigned int frameIndex)
{
	cv::Mat image = cv::imread(StringEx::Combine(TrafficDirectory::DataDir, "images/", _channelIndex, "_", frameIndex, ".jpg"));

	if (image.empty())
	{
		return;
	}
	//车道
	for (unsigned int i = 0; i < _laneCaches.size(); ++i)
	{
		FlowLaneCache& cache = _laneCaches[i];
		//流量区域
		ImageDrawing::DrawPolygon(&image, cache.FlowRegion, cv::Scalar(0, 0, 255));
		//排队区域
		ImageDrawing::DrawPolygon(&image, cache.QueueRegion, cv::Scalar(0, 0, 200));
	}
	//检测项
	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		cv::Scalar carScalar;
		//绿色新车
		if (it->second.Status == DetectStatus::New)
		{
			carScalar = cv::Scalar(0, 255, 0);
		}
		//黄色在区域
		else if (it->second.Status == DetectStatus::In)
		{
			carScalar = cv::Scalar(0, 255, 255);
		}
		else
		{
			continue;
		}
		ImageDrawing::DrawRectangle(&image, it->second.Region, carScalar);
		ImageDrawing::DrawText(&image, StringEx::Combine(it->first.substr(it->first.size() - 4, 4), "-", static_cast<int>(it->second.Type)), it->second.Region.HitPoint(), carScalar);
	}
	_detectImage.BgrToJpgFile(image.data, _width, _height, StringEx::Combine(TrafficDirectory::TempDir,_channelIndex,"_",frameIndex,".jpg"));
}
