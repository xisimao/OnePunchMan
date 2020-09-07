#include "FlowDetector.h"

using namespace std;
using namespace OnePunchMan;

const string FlowDetector::IOTopic("IO");
const string FlowDetector::FlowTopic("Flow");
const string FlowDetector::VideoStructTopic("VideoStruct");
const int FlowDetector::ReportMaxSpan = 60 * 1000;
const int FlowDetector::DeleteSpan = 60 * 1000;

FlowDetector::FlowDetector(int width, int height, MqttChannel* mqtt)
	:TrafficDetector(width, height, mqtt), _taskId(0), _lastFrameTimeStamp(0), _currentMinuteTimeStamp(0), _nextMinuteTimeStamp(0)
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

void FlowDetector::UpdateChannel(const unsigned char taskId, const FlowChannel& channel)
{
	vector<FlowLaneCache> lanes;
	string regionsParam;
	regionsParam.append("[");
	for (vector<FlowLane>::const_iterator it = channel.Lanes.begin(); it != channel.Lanes.end(); ++it)
	{
		FlowLaneCache laneCache;
		Line detectLine = Line::FromJson(it->DetectLine);
		Line stopLine = Line::FromJson(it->StopLine);
		laneCache.Region = Polygon::FromJson(it->Region);
		if (detectLine.Empty()
			|| stopLine.Empty()
			|| laneCache.Region.Empty())
		{
			LogPool::Warning(LogEvent::Detect, "line empty channel:", it->ChannelIndex, "lane:", it->LaneId);
		}
		else
		{
			double pixels = detectLine.Middle().Distance(stopLine.Middle());
			laneCache.LaneId = it->LaneId;
			laneCache.LaneName = it->LaneName;
			laneCache.Direction = it->Direction;
			laneCache.MeterPerPixel = it->Length / pixels;
			lanes.push_back(laneCache);
			regionsParam.append(it->Region);
			regionsParam.append(",");
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

	lock_guard<mutex> detectLock(_laneMutex);
	_taskId = taskId;
	_laneCaches.assign(lanes.begin(), lanes.end());
	_channelUrl = channel.ChannelUrl;

	_lanesInited = !_laneCaches.empty() && _laneCaches.size() == channel.Lanes.size();
	if (!_lanesInited)
	{
		LogPool::Warning("车道未初始化，通道序号:", channel.ChannelIndex);
	}
	_channelIndex = channel.ChannelIndex;
	if (channel.GlobalDetect)
	{
		_param = GetDetectParam();
	}
	else
	{
		_param = GetDetectParam(regionsParam);
	}
	_setParam = false;
	_writeBmp = true;
	remove(StringEx::Combine("../images/channel_", channel.ChannelIndex, ".bmp").c_str());
	//输出图片时先删除旧的
	_outputImage = channel.OutputImage;
	if (_outputImage)
	{
		Command::Execute(StringEx::Combine("rm -rf ../temp/jpg_", channel.ChannelIndex, "*"));
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
	_currentMinuteTimeStamp = DateTime(date.Year(), date.Month(), date.Day(), date.Hour(), date.Minute(), 0).UtcTimeStamp();
	_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
	_outputRecogn = channel.OutputRecogn;
	LogPool::Information(LogEvent::Flow, "初始化流量检测，通道序号:", channel.ChannelIndex, "任务号:", _taskId, "是否输出检测报告:", channel.OutputReport, "是否输出检测图片:", channel.OutputImage, "是否输出识别数据:", channel.OutputRecogn, "是否全局检测:", channel.GlobalDetect, "当前检测周期时间戳:", _currentMinuteTimeStamp, "下一个检测周期时间戳:", _nextMinuteTimeStamp);
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
	for (vector<FlowReportCache>::iterator it = _reportCaches.begin(); it != _reportCaches.end(); ++it)
	{
		string flowJson;
		JsonSerialization::SerializeValue(&flowJson, "minute", it->Minute);
		JsonSerialization::SerializeValue(&flowJson, "laneId", it->LaneId);
		JsonSerialization::SerializeValue(&flowJson, "laneName", it->LaneName);
		JsonSerialization::SerializeValue(&flowJson, "direction", it->Direction);
		JsonSerialization::SerializeValue(&flowJson, "persons", it->Persons);
		JsonSerialization::SerializeValue(&flowJson, "bikes", it->Bikes);
		JsonSerialization::SerializeValue(&flowJson, "motorcycles", it->Motorcycles);
		JsonSerialization::SerializeValue(&flowJson, "cars", it->Cars);
		JsonSerialization::SerializeValue(&flowJson, "tricycles", it->Tricycles);
		JsonSerialization::SerializeValue(&flowJson, "buss", it->Buss);
		JsonSerialization::SerializeValue(&flowJson, "vans", it->Vans);
		JsonSerialization::SerializeValue(&flowJson, "trucks", it->Trucks);
		JsonSerialization::SerializeValue(&flowJson, "averageSpeed", static_cast<int>(it->Speed));
		JsonSerialization::SerializeValue(&flowJson, "headDistance", it->HeadDistance);
		JsonSerialization::SerializeValue(&flowJson, "headSpace", it->HeadSpace);
		JsonSerialization::SerializeValue(&flowJson, "timeOccupancy", static_cast<int>(it->TimeOccupancy));
		JsonSerialization::SerializeValue(&flowJson, "trafficStatus", it->TrafficStatus);
		JsonSerialization::AddClassItem(&flowsJson, flowJson);
	}
	string vehiclesJson;
	for (vector<VideoStruct_Vehicle>::iterator it = _vehicleReportCaches.begin(); it != _vehicleReportCaches.end(); ++it)
	{
		string vehicleJson;
		JsonSerialization::SerializeValue(&vehicleJson, "time", StringEx::Combine(it->Minute, ":", it->Second));
		JsonSerialization::SerializeValue(&vehicleJson, "laneId", it->LaneId);
		JsonSerialization::SerializeValue(&vehicleJson, "laneName", it->LaneName);		
		JsonSerialization::SerializeValue(&vehicleJson, "direction", it->Direction);		
		JsonSerialization::SerializeValue(&vehicleJson, "carType", it->CarType);
		JsonSerialization::SerializeValue(&vehicleJson, "carColor", it->CarColor);
		JsonSerialization::SerializeValue(&vehicleJson, "carBrand", it->CarBrand);
		JsonSerialization::SerializeValue(&vehicleJson, "plateType", it->PlateType);
		JsonSerialization::SerializeValue(&vehicleJson, "plateNumber", it->PlateNumber);
		JsonSerialization::AddClassItem(&vehiclesJson, vehicleJson);
	}
	string bikesJson;
	for (vector<VideoStruct_Bike>::iterator it = _bikeReportCaches.begin(); it != _bikeReportCaches.end(); ++it)
	{
		string bikeJson;
		JsonSerialization::SerializeValue(&bikeJson, "time", StringEx::Combine(it->Minute, ":", it->Second));
		JsonSerialization::SerializeValue(&bikeJson, "laneId", it->LaneId);
		JsonSerialization::SerializeValue(&bikeJson, "laneName", it->LaneName);
		JsonSerialization::SerializeValue(&bikeJson, "direction", it->Direction);
		JsonSerialization::SerializeValue(&bikeJson, "bikeType", it->BikeType);
		JsonSerialization::AddClassItem(&bikesJson, bikeJson);
	}
	string pedestrainsJson;
	for (vector<VideoStruct_Pedestrain>::iterator it = _pedestrainReportCaches.begin(); it != _pedestrainReportCaches.end(); ++it)
	{
		string pedestrainJson;
		JsonSerialization::SerializeValue(&pedestrainJson, "time", StringEx::Combine(it->Minute, ":", it->Second));
		JsonSerialization::SerializeValue(&pedestrainJson, "laneId", it->LaneId);
		JsonSerialization::SerializeValue(&pedestrainJson, "laneName", it->LaneName);
		JsonSerialization::SerializeValue(&pedestrainJson, "direction", it->Direction);
		JsonSerialization::SerializeValue(&pedestrainJson, "sex", it->Sex);
		JsonSerialization::SerializeValue(&pedestrainJson, "age", it->Age);
		JsonSerialization::SerializeValue(&pedestrainJson, "upperColor", it->UpperColor);
		JsonSerialization::AddClassItem(&pedestrainsJson, pedestrainJson);
	}
	JsonSerialization::SerializeClass(json, "flows", flowsJson);
	JsonSerialization::SerializeClass(json, "vehicles", vehiclesJson);
	JsonSerialization::SerializeClass(json, "bikes", bikesJson);
	JsonSerialization::SerializeClass(json, "pedestrains", pedestrainsJson);
}

void FlowDetector::CalculateMinuteFlow(FlowLaneCache* laneCache)
{
	//平均速度(km/h)
	laneCache->Speed = laneCache->TotalTime == 0 ? 0 : (laneCache->TotalDistance * laneCache->MeterPerPixel / 1000.0) / (static_cast<double>(laneCache->TotalTime) / 3600000.0);
	//车道时距(sec)
	laneCache->HeadDistance = laneCache->Vehicles > 1 ? static_cast<double>(laneCache->TotalSpan) / static_cast<double>(laneCache->Vehicles - 1) / 1000.0 : 0;
	//车头间距(m)
	laneCache->HeadSpace = laneCache->Speed * 1000 * laneCache->HeadDistance / 3600.0;
	//时间占用率(%)
	laneCache->TimeOccupancy = static_cast<double>(laneCache->TotalInTime) / 60000.0 * 100;
	if (laneCache->Speed > 40)
	{
		laneCache->TrafficStatus = static_cast<int>(TrafficStatus::Good);
	}
	else if (laneCache->Speed <= 40 && laneCache->Speed > 30)
	{
		laneCache->TrafficStatus = static_cast<int>(TrafficStatus::Normal);
	}
	else if (laneCache->Speed <= 30 && laneCache->Speed > 20)
	{
		laneCache->TrafficStatus = static_cast<int>(TrafficStatus::Warning);
	}
	else if (laneCache->Speed <= 20 && laneCache->Speed > 15)
	{
		laneCache->TrafficStatus = static_cast<int>(TrafficStatus::Warning);
	}
	else
	{
		laneCache->TrafficStatus = static_cast<int>(TrafficStatus::Dead);
	}
}

void FlowDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, string* param, unsigned char taskId, const unsigned char* iveBuffer, unsigned int frameIndex, unsigned char frameSpan)
{
	if (_taskId != taskId)
	{
		return;
	}
	lock_guard<mutex> detectLock(_laneMutex);
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
			CalculateMinuteFlow(&laneCache);
			string laneJson;
			JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&laneJson, "laneId", laneCache.LaneId);
			JsonSerialization::SerializeValue(&laneJson, "timeStamp", _currentMinuteTimeStamp);
			JsonSerialization::SerializeValue(&laneJson, "persons", laneCache.Persons);
			JsonSerialization::SerializeValue(&laneJson, "bikes", laneCache.Bikes);
			JsonSerialization::SerializeValue(&laneJson, "motorcycles", laneCache.Motorcycles);
			JsonSerialization::SerializeValue(&laneJson, "cars", laneCache.Cars);
			JsonSerialization::SerializeValue(&laneJson, "tricycles", laneCache.Tricycles);
			JsonSerialization::SerializeValue(&laneJson, "buss", laneCache.Buss);
			JsonSerialization::SerializeValue(&laneJson, "vans", laneCache.Vans);
			JsonSerialization::SerializeValue(&laneJson, "trucks", laneCache.Trucks);

			JsonSerialization::SerializeValue(&laneJson, "averageSpeed", static_cast<int>(laneCache.Speed));
			JsonSerialization::SerializeValue(&laneJson, "headDistance", laneCache.HeadDistance);
			JsonSerialization::SerializeValue(&laneJson, "headSpace", laneCache.HeadSpace);
			JsonSerialization::SerializeValue(&laneJson, "timeOccupancy", static_cast<int>(laneCache.TimeOccupancy));
			JsonSerialization::SerializeValue(&laneJson, "trafficStatus", laneCache.TrafficStatus);
			JsonSerialization::AddClassItem(&flowLanesJson, laneJson);

			LogPool::Debug(LogEvent::Detect, "lane:", laneCache.LaneId, "vehicles:", laneCache.Cars + laneCache.Tricycles + laneCache.Buss + laneCache.Vans + laneCache.Trucks, "bikes:", laneCache.Bikes + laneCache.Motorcycles, "persons:", laneCache.Persons, laneCache.Speed, "km/h ", laneCache.HeadDistance, "sec ", laneCache.TimeOccupancy, "%");
			if (_outputReport)
			{
				FlowReportCache reportCache;
				reportCache.Minute = _currentReportMinute;
				reportCache.LaneId = laneCache.LaneId;
				reportCache.LaneName = laneCache.LaneName;
				reportCache.Direction = laneCache.Direction;
				reportCache.Bikes = laneCache.Bikes;
				reportCache.Buss = laneCache.Buss;
				reportCache.Cars = laneCache.Cars;
				reportCache.Motorcycles = laneCache.Motorcycles;
				reportCache.Persons = laneCache.Persons;
				reportCache.Tricycles = laneCache.Tricycles;
				reportCache.Trucks = laneCache.Trucks;
				reportCache.Vans = laneCache.Vans;
				reportCache.HeadDistance = laneCache.HeadDistance;
				reportCache.HeadSpace = laneCache.HeadSpace;
				reportCache.Speed = laneCache.Speed;
				reportCache.TimeOccupancy = laneCache.TimeOccupancy;
				reportCache.TrafficStatus = laneCache.TrafficStatus;		
				_reportCaches.push_back(reportCache);
			}
			laneCache.Persons = 0;
			laneCache.Bikes = 0;
			laneCache.Motorcycles = 0;
			laneCache.Tricycles = 0;
			laneCache.Trucks = 0;
			laneCache.Vans = 0;
			laneCache.Cars = 0;
			laneCache.Buss = 0;

			laneCache.TotalDistance = 0.0;
			laneCache.TotalTime = 0;

			laneCache.TotalInTime = 0;

			laneCache.LastInRegion = 0;
			laneCache.Vehicles = 0;
			laneCache.TotalSpan = 0;
		}

		if (!flowLanesJson.empty()
			&& _mqtt != NULL
			&& !_outputReport
			&& (timeStamp - _nextMinuteTimeStamp) < ReportMaxSpan)
		{
			_mqtt->Send(FlowTopic, flowLanesJson);
		}

		//结算后认为该分钟结束，当前帧收到的数据结算到下一分钟
		DateTime currentTime=DateTime::ParseTimeStamp(timeStamp);
		DateTime currentTimePoint(currentTime.Year(), currentTime.Month(), currentTime.Day(), currentTime.Hour(), currentTime.Minute(), 0);
		_currentMinuteTimeStamp = currentTimePoint.UtcTimeStamp();
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
		bool ioStatus = false;
		for (map<string, DetectItem>::iterator dit = detectItems->begin(); dit != detectItems->end(); ++dit)
		{
			if (dit->second.Status != DetectStatus::Out)
			{
				continue;
			}
			if (laneCache.Region.Contains(dit->second.Region.HitPoint()))
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
						laneCache.Vehicles += 1;
					}
					else if (dit->second.Type == DetectType::Tricycle)
					{
						laneCache.Tricycles += 1;
						laneCache.Vehicles += 1;
					}
					else if (dit->second.Type == DetectType::Bus)
					{
						laneCache.Buss += 1;
						laneCache.Vehicles += 1;
					}
					else if (dit->second.Type == DetectType::Van)
					{
						laneCache.Vans += 1;
						laneCache.Vehicles += 1;
					}
					else if (dit->second.Type == DetectType::Truck)
					{
						laneCache.Trucks += 1;
						laneCache.Vehicles += 1;
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
		}

		//如果上一次有车，则认为到这次检测为止都有车
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
			LogPool::Debug(LogEvent::Detect, "lane:", laneCache.LaneId, "io:", ioStatus);
		}
	}
	_lastFrameTimeStamp = timeStamp;
	if (!ioLanesJson.empty()
		&& _mqtt != NULL
		&& !_outputReport)
	{
		_mqtt->Send(IOTopic, ioLanesJson);
	}
	DrawDetect(*detectItems, iveBuffer, frameIndex);
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
			CalculateMinuteFlow(&cache);

			FlowReportCache reportCache;
			reportCache.Minute = _currentReportMinute;
			reportCache.LaneId = cache.LaneId;
			reportCache.Direction = cache.Direction;
			reportCache.Bikes = cache.Bikes;
			reportCache.Buss = cache.Buss;
			reportCache.Cars = cache.Cars;
			reportCache.Motorcycles = cache.Motorcycles;
			reportCache.Persons = cache.Persons;
			reportCache.Tricycles = cache.Tricycles;
			reportCache.Trucks = cache.Trucks;
			reportCache.Vans = cache.Vans;
			reportCache.HeadDistance = cache.HeadDistance;
			reportCache.HeadSpace = cache.HeadSpace;
			reportCache.Speed = cache.Speed;
			reportCache.TimeOccupancy = cache.TimeOccupancy;
			reportCache.TrafficStatus = cache.TrafficStatus;
			reportCache.LaneName = cache.LaneName;
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
			JsonSerialization::SerializeValue(&json, "timeStamp", DateTime::UtcNowTimeStamp());
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

			LogPool::Debug(LogEvent::Detect, "lane:", it->LaneId, "type:", recognItem.Type);
			return;
		}
	}
	LogPool::Information(LogEvent::Detect, "识别项丢失，识别项编号：", recognItem.Guid, "识别项类型：", recognItem.Type, "帧序号：", recognItem.FrameIndex);
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
			JsonSerialization::SerializeValue(&json, "timeStamp", DateTime::UtcNowTimeStamp());
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

			LogPool::Debug(LogEvent::Detect, "lane:",it->LaneId, "type:", recognItem.Type);
			return;
		}
	}
	LogPool::Information(LogEvent::Detect, "识别项丢失，识别项编号：", recognItem.Guid, "识别项类型：", recognItem.Type, "帧序号：", recognItem.FrameIndex);
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
			JsonSerialization::SerializeValue(&json, "timeStamp", DateTime::UtcNowTimeStamp());
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
			LogPool::Debug(LogEvent::Detect, "lane:", it->LaneId, "type:", recognItem.Type);
			return;
		}
	}
	LogPool::Information(LogEvent::Detect, "识别项丢失，识别项编号：", recognItem.Guid, "识别项类型：", recognItem.Type, "帧序号：", recognItem.FrameIndex);
}

void FlowDetector::DrawDetect(const map<string, DetectItem>& detectItems,const unsigned char* iveBuffer, unsigned int frameIndex)
{
	if (!_outputImage)
	{
		return;
	}
	bool isKeyFrame = false;
	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		if (it->second.Status == DetectStatus::New)
		{
			isKeyFrame = true;
			break;
		}
	}
	if (isKeyFrame)
	{
		ImageConvert::IveToBgr(iveBuffer, _width, _height, _detectBgrBuffer);
		cv::Mat image(_height, _width, CV_8UC3, _detectBgrBuffer);
		for (unsigned int i = 0; i < _laneCaches.size(); ++i)
		{
			FlowLaneCache& cache = _laneCaches[i];
			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
		}
		for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
		{
			cv::Scalar scalar;
			//绿色新车
			if (it->second.Status == DetectStatus::New)
			{
				scalar = cv::Scalar(0, 255, 0);
				cv::Scalar textScalar;
				if (it->second.Type == DetectType::Pedestrain)
				{
					textScalar = cv::Scalar(220, 20, 60);
				}
				else if(it->second.Type==DetectType::Bike || it->second.Type == DetectType::Motobike)
				{
					textScalar = cv::Scalar(255, 255, 0);
				}
				else
				{
					textScalar = cv::Scalar(124, 252, 0);
				}
				ImageConvert::DrawText(&image, it->first.substr(it->first.size() - 4, 4), it->second.Region.HitPoint(), textScalar);
			}
			//黄色在区域
			else if (it->second.Status == DetectStatus::In)
			{
				scalar = cv::Scalar(0, 255, 255);
			}
			//蓝色不在区域
			else
			{
				scalar = cv::Scalar(255, 0, 0);
			}
			ImageConvert::DrawPoint(&image, it->second.Region.HitPoint(), scalar);
			
		}
		int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, _detectJpgBuffer, _jpgSize);
		ImageConvert::JpgToFile(_detectJpgBuffer, jpgSize, _channelIndex, frameIndex);
	}
}


