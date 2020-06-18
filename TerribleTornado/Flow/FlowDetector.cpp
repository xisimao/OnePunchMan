#include "FlowDetector.h"

using namespace std;
using namespace OnePunchMan;

const string FlowDetector::IOTopic("IO");
const string FlowDetector::FlowTopic("Flow");
const string FlowDetector::VideoStructTopic("VideoStruct");
const int FlowDetector::ReportMaxSpan = 60 * 1000;

FlowDetector::FlowDetector(int width, int height, MqttChannel* mqtt)
	:TrafficDetector(width,height, mqtt), _taskId(0),_lastFrameTimeStamp(0), _currentMinuteTimeStamp(0),_nextMinuteTimeStamp(0), _outputImage(false),_report(NULL)
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

void FlowDetector::UpdateChannel(unsigned char taskId,const FlowChannel& channel)
{
	vector<FlowLaneCache> lanes;
	string regionsParam;
	regionsParam.append("[");
	for (vector<FlowLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
	{
		FlowLaneCache cache;
		Line detectLine = Line::FromJson(lit->DetectLine);
		Line stopLine = Line::FromJson(lit->StopLine);
		cache.Region = Polygon::FromJson(lit->Region);
		if (detectLine.Empty()
			|| stopLine.Empty()
			|| cache.Region.Empty())
		{
			LogPool::Warning(LogEvent::Detect, "line empty channel:", lit->ChannelIndex, "lane:", lit->LaneId);
		}
		else
		{
			double pixels = detectLine.Middle().Distance(stopLine.Middle());
			cache.LaneId = lit->LaneId;
			cache.MeterPerPixel = lit->Length / pixels;
			lanes.push_back(cache);
			regionsParam.append(lit->Region);
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

	unique_lock<mutex> detectLock(_detectLaneMutex);
	_taskId = taskId;
	_detectLanes.assign(lanes.begin(), lanes.end());
	_channelUrl = channel.ChannelUrl;

	_lanesInited = !_detectLanes.empty() && _detectLanes.size() == channel.Lanes.size();
	if (!_lanesInited)
	{
		LogPool::Warning("lane init failed", channel.ChannelIndex);
	}
	_channelIndex = channel.ChannelIndex;
	_param = GetDetectParam(regionsParam);
	_setParam = false;
	//输出图片时先删除旧的
	_outputImage = channel.OutputImage;
	if (_outputImage)
	{
		Command::Execute(StringEx::Combine("rm -rf ../temp/jpg_",channel.ChannelIndex,"*"));
	}

	//输出报告时调整时间戳范围
	if (_report != NULL)
	{
		delete _report;
		_report = NULL;
	}
	DateTime date;
	if (channel.OutputReport)
	{
		_report = new ReportWriter(channel.ChannelIndex,static_cast<int>(channel.Lanes.size()));
		date = DateTime(0);
	}
	else
	{
		date = DateTime::Now();
	}
	_currentMinuteTimeStamp = DateTime(date.Year(), date.Month(), date.Day(), date.Hour(), date.Minute(), 0).UtcTimeStamp();
	_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;

	LogPool::Information(LogEvent::Flow, "channel:",channel.ChannelIndex,"task:",_taskId,"output image:",channel.OutputImage,"output report:",channel.OutputReport,"current:",_currentMinuteTimeStamp,"next:", _nextMinuteTimeStamp);
	detectLock.unlock();

	lock_guard<mutex> recognLock(_recognLaneMutex);
	_recognLanes.assign(lanes.begin(), lanes.end());
	_recognChannelUrl = channel.ChannelUrl;
	
}

void FlowDetector::ClearChannel()
{
	unique_lock<mutex> detectLock(_detectLaneMutex);
	_taskId = 0;
	_detectLanes.clear();
	_channelUrl = string();
	_lanesInited = false;
	detectLock.unlock();

	lock_guard<mutex> recognLock(_recognLaneMutex);
	_recognLanes.clear();
	_recognChannelUrl = string();
}

void FlowDetector::CalculateMinuteFlow(FlowLaneCache* laneCache)
{
	//平均速度(km/h)
	laneCache->Speed = laneCache->TotalTime == 0 ? 0 : (laneCache->TotalDistance * laneCache->MeterPerPixel / 1000.0) / (static_cast<double>(laneCache->TotalTime) / 3600000.0);
	//车道时距(sec)
	laneCache->HeadDistance = laneCache->Vehicles > 1 ? static_cast<double>(laneCache->TotalSpan) / static_cast<double>(laneCache->Vehicles - 1) / 1000.0 : 0;
	//车头间距(m)
	laneCache->HeadSpace = laneCache->Speed * 1000 * laneCache->HeadDistance/ 3600.0;
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
	lock_guard<mutex> detectLock(_detectLaneMutex);
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}
	//输出检测报告时调整时间戳
	if (_report != NULL)
	{
		timeStamp = frameIndex * frameSpan;
	}
	//结算上一分钟
	if (timeStamp > _nextMinuteTimeStamp)
	{
		LogPool::Information(LogEvent::Flow, "flow",timeStamp,_nextMinuteTimeStamp);
		string flowLanesJson;
		for (unsigned int i = 0; i < _detectLanes.size(); ++i)
		{
			FlowLaneCache& cache = _detectLanes[i];
			CalculateMinuteFlow(&cache);
			string laneJson;
			JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&laneJson, "laneId", cache.LaneId);
			JsonSerialization::SerializeValue(&laneJson, "timeStamp", _currentMinuteTimeStamp);
			JsonSerialization::SerializeValue(&laneJson, "persons", cache.Persons);
			JsonSerialization::SerializeValue(&laneJson, "bikes", cache.Bikes);
			JsonSerialization::SerializeValue(&laneJson, "motorcycles", cache.Motorcycles);
			JsonSerialization::SerializeValue(&laneJson, "cars", cache.Cars);
			JsonSerialization::SerializeValue(&laneJson, "tricycles", cache.Tricycles);
			JsonSerialization::SerializeValue(&laneJson, "buss", cache.Buss);
			JsonSerialization::SerializeValue(&laneJson, "vans", cache.Vans);
			JsonSerialization::SerializeValue(&laneJson, "trucks", cache.Trucks);

			JsonSerialization::SerializeValue(&laneJson, "averageSpeed", static_cast<int>(cache.Speed));
			JsonSerialization::SerializeValue(&laneJson, "headDistance", cache.HeadDistance);
			JsonSerialization::SerializeValue(&laneJson, "headSpace", cache.HeadSpace);
			JsonSerialization::SerializeValue(&laneJson, "timeOccupancy", static_cast<int>(cache.TimeOccupancy));
			JsonSerialization::SerializeValue(&laneJson, "trafficStatus", cache.TrafficStatus);
			JsonSerialization::AddClassItem(&flowLanesJson, laneJson);

			LogPool::Debug(LogEvent::Detect, "lane:", cache.LaneId, "vehicles:", cache.Cars + cache.Tricycles + cache.Buss + cache.Vans + cache.Trucks, "bikes:", cache.Bikes + cache.Motorcycles, "persons:", cache.Persons, cache.Speed, "km/h ", cache.HeadDistance, "sec ", cache.TimeOccupancy, "%");
			if (_report != NULL)
			{
				_report->Write(cache);
			}
			cache.Persons = 0;
			cache.Bikes = 0;
			cache.Motorcycles = 0;
			cache.Tricycles = 0;
			cache.Trucks = 0;
			cache.Vans = 0;
			cache.Cars = 0;
			cache.Buss = 0;

			cache.TotalDistance = 0.0;
			cache.TotalTime = 0;

			cache.TotalInTime = 0;

			cache.LastInRegion = 0;
			cache.Vehicles = 0;
			cache.TotalSpan = 0;
		}

		if (!flowLanesJson.empty()
			&& _mqtt != NULL
			&& _report==NULL
			&& (timeStamp - _nextMinuteTimeStamp) < ReportMaxSpan)
		{
			_mqtt->Send(FlowTopic, flowLanesJson);
		}

		//结算后认为该分钟结束，当前帧收到的数据结算到下一分钟
		DateTime currentTime(timeStamp);
		DateTime currentTimePoint(currentTime.Year(), currentTime.Month(), currentTime.Day(), currentTime.Hour(), currentTime.Minute(), 0);
		_currentMinuteTimeStamp = currentTimePoint.UtcTimeStamp();
		_lastFrameTimeStamp = _currentMinuteTimeStamp;
		_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
	}

	//计算当前帧
	string ioLanesJson;
	for (unsigned int i = 0; i < _detectLanes.size(); ++i)
	{
		FlowLaneCache& cache = _detectLanes[i];
		cache.CurrentItems()->clear();

		bool ioStatus = false;
		for (map<string, DetectItem>::iterator it = detectItems->begin(); it != detectItems->end(); ++it)
		{
			if (it->second.Status != DetectStatus::Out)
			{
				continue;
			}
			if (cache.Region.Contains(it->second.Region.HitPoint()))
			{
				ioStatus = true;
				map<string, FlowDetectCache>::const_iterator mit = cache.LastItems()->find(it->first);
				//如果是新车则计算流量和车头时距
				//流量=车数量
				//车头时距=所有进入区域的时间差的平均值
				if (mit == cache.LastItems()->end())
				{
					it->second.Status = DetectStatus::New;
					if (it->second.Type == DetectType::Car)
					{
						cache.Cars += 1;
						cache.Vehicles += 1;
					}
					else if (it->second.Type == DetectType::Tricycle)
					{
						cache.Tricycles += 1;
						cache.Vehicles += 1;
					}
					else if (it->second.Type == DetectType::Bus)
					{
						cache.Buss += 1;
						cache.Vehicles += 1;
					}
					else if (it->second.Type == DetectType::Van)
					{
						cache.Vans += 1;
						cache.Vehicles += 1;
					}
					else if (it->second.Type == DetectType::Truck)
					{
						cache.Trucks += 1;
						cache.Vehicles += 1;
					}
					else if (it->second.Type == DetectType::Bike)
					{
						cache.Bikes += 1;
					}
					else if (it->second.Type == DetectType::Motobike)
					{
						cache.Motorcycles += 1;
					}
					else if (it->second.Type == DetectType::Pedestrain)
					{
						cache.Persons += 1;
					}
					if (cache.LastInRegion != 0)
					{
						cache.TotalSpan += timeStamp - cache.LastInRegion;
					}
					cache.LastInRegion = timeStamp;
				}
				//如果是已经记录的车则计算平均速度
				//平均速度=总距离/总时间
				//总距离=两次检测到的点的距离*每个像素代表的米数
				//总时间=两次检测到的时间戳时长
				else
				{
					it->second.Status = DetectStatus::In;
					double distance = it->second.Region.HitPoint().Distance(mit->second.HitPoint);
					cache.TotalDistance += distance;
					cache.TotalTime += timeStamp - _lastFrameTimeStamp;
				}
				FlowDetectCache detectCache;
				detectCache.HitPoint = it->second.Region.HitPoint();
				cache.CurrentItems()->insert(pair<string, FlowDetectCache>(it->first, detectCache));
			}
		}

		//如果上一次有车，则认为到这次检测为止都有车
		//时间占有率=总时间/一分钟
		if (!cache.LastItems()->empty())
		{
			cache.TotalInTime += timeStamp - _lastFrameTimeStamp;
		}

		cache.SwitchFlag();

		if (ioStatus != cache.IoStatus)
		{
			cache.IoStatus = ioStatus;
			string laneJson;
			JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&laneJson, "laneId", cache.LaneId);
			JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
			JsonSerialization::SerializeValue(&laneJson, "status", (int)ioStatus);
			JsonSerialization::AddClassItem(&ioLanesJson, laneJson);
			LogPool::Debug(LogEvent::Detect, "lane:", cache.LaneId, "io:", ioStatus);
		}
	}
	_lastFrameTimeStamp = timeStamp;
	if (!ioLanesJson.empty()
		&& _mqtt != NULL
		&&_report==NULL)
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
	lock_guard<mutex> lock(_detectLaneMutex);
	for (unsigned int i = 0; i < _detectLanes.size(); ++i)
	{
		FlowLaneCache& cache = _detectLanes[i];
		CalculateMinuteFlow(&cache);
		if (_report != NULL)
		{
			_report->Write(cache);
		}
		
	}
	delete _report;
	_report = NULL;
}

void FlowDetector::HandleRecogn(string* json,const RecognItem& recognItem, const unsigned char* iveBuffer)
{
	long long timeStamp = DateTime::UtcNowTimeStamp();
	lock_guard<mutex> recognLock(_recognLaneMutex);
	for (unsigned int i = 0; i < _recognLanes.size(); ++i)
	{
		if (_recognLanes[i].Region.Contains(recognItem.Region.HitPoint()))
		{
			JsonSerialization::SerializeValue(json, "channelUrl", _recognChannelUrl);
			JsonSerialization::SerializeValue(json, "laneId", _recognLanes[i].LaneId);
			JsonSerialization::SerializeValue(json, "timeStamp", timeStamp);
			string image;
			ImageConvert::IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height, _recognBgrBuffer, &image, _recognJpgBuffer, _jpgSize);
			JsonSerialization::SerializeValue(json, "image", image);
			if (_mqtt != NULL && _report == NULL)
			{
				_mqtt->Send(VideoStructTopic, *json);
			}
			LogPool::Debug(LogEvent::Detect, "lane:", _recognLanes[i].LaneId, "type:", recognItem.Type);
			return;
		}
	}
}

void FlowDetector::HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Vehicle& vehicle)
{
	string json;
	JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Vehicle);
	JsonSerialization::SerializeValue(&json, "carType", vehicle.CarType);
	JsonSerialization::SerializeValue(&json, "carColor", vehicle.CarColor);
	JsonSerialization::SerializeValue(&json, "carBrand", vehicle.CarBrand);
	JsonSerialization::SerializeValue(&json, "plateType", vehicle.PlateType);
	JsonSerialization::SerializeValue(&json, "plateNumber", vehicle.PlateNumber);
	HandleRecogn(&json, recognItem, iveBuffer);
}

void FlowDetector::HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Bike& bike)
{
	string json;
	JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Bike);
	JsonSerialization::SerializeValue(&json, "bikeType", bike.BikeType);
	HandleRecogn(&json, recognItem, iveBuffer);
}

void FlowDetector::HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, const VideoStruct_Pedestrain& pedestrain)
{
	string json;
	JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Pedestrain);
	JsonSerialization::SerializeValue(&json, "sex", pedestrain.Sex);
	JsonSerialization::SerializeValue(&json, "age", pedestrain.Age);
	JsonSerialization::SerializeValue(&json, "upperColor", pedestrain.UpperColor);
	HandleRecogn(&json, recognItem, iveBuffer);
}

void FlowDetector::DrawDetect(const map<string, DetectItem>& detectItems, const unsigned char* iveBuffer, unsigned int frameIndex)
{
	if (!_outputImage)
	{
		return;
	}
	bool hasNew = false;
	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		if (it->second.Status == DetectStatus::New)
		{
			hasNew = true;
			break;
		}
	}
	if (hasNew)
	{
		ImageConvert::IveToBgr(iveBuffer, _width, _height, _detectBgrBuffer);
		cv::Mat image(_height, _width, CV_8UC3, _detectBgrBuffer);
		for (unsigned int i = 0; i < _detectLanes.size(); ++i)
		{
			FlowLaneCache& cache = _detectLanes[i];
			ImageConvert::DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
		}
		for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
		{
			cv::Point point(it->second.Region.HitPoint().X, it->second.Region.HitPoint().Y);
			cv::Scalar scalar;
			//绿色新车
			if (it->second.Status == DetectStatus::New)
			{
				scalar = cv::Scalar(0, 255, 0);
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
			cv::circle(image, point, 10, scalar, -1);
		}

		int jpgSize = ImageConvert::BgrToJpg(image.data, _width, _height, _detectJpgBuffer, _jpgSize);
		ImageConvert::JpgToFile(_detectJpgBuffer, jpgSize, _channelIndex, frameIndex);
	}

}

