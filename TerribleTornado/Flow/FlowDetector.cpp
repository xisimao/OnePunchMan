#include "FlowDetector.h"

using namespace std;
using namespace OnePunchMan;

const string FlowDetector::IOTopic("IO");
const string FlowDetector::FlowTopic("Flow");
const string FlowDetector::VideoStructTopic("VideoStruct");

FlowDetector::FlowDetector(int width, int height, MqttChannel* mqtt, bool debug)
	:TrafficDetector(width,height, mqtt,debug), _lastFrameTimeStamp(0)
	
{
	DateTime now = DateTime::Now();
	_currentMinuteTimeStamp = DateTime(now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), 0).UtcTimeStamp();
	_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
}

void FlowDetector::UpdateChannel(const FlowChannel& channel)
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
	_detectLanes.assign(lanes.begin(), lanes.end());
	_channelUrl = channel.ChannelUrl;

	_lanesInited = !_detectLanes.empty() && _detectLanes.size() == channel.Lanes.size();
	if (!_lanesInited)
	{
		LogPool::Warning("lane init failed", channel.ChannelIndex);
	}
	_channelIndex = channel.ChannelIndex;
	_param = StringEx::Combine("{\"Detect\":{\"DetectRegion\":", regionsParam, ",\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}");
	_setParam = false;
	detectLock.unlock();

	lock_guard<mutex> recognLock(_recognLaneMutex);
	_recognLanes.assign(lanes.begin(), lanes.end());
	_recognChannelUrl = channel.ChannelUrl;
	
}

void FlowDetector::ClearChannel()
{
	unique_lock<mutex> detectLock(_detectLaneMutex);
	_detectLanes.clear();
	_channelUrl = string();
	_lanesInited = false;
	detectLock.unlock();

	lock_guard<mutex> recognLock(_recognLaneMutex);
	_recognLanes.clear();
	_recognChannelUrl = string();
}

void FlowDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, string* param, const unsigned char* iveBuffer, int frameIndex, int frameSpan)
{
	if (_debug)
	{
		timeStamp = frameIndex * frameSpan;
	}
	lock_guard<mutex> detectLock(_detectLaneMutex);
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}
	//������һ����
	if (timeStamp > _nextMinuteTimeStamp)
	{
		string flowLanesJson;
		for (unsigned int i = 0; i < _detectLanes.size(); ++i)
		{
			FlowLaneCache& cache = _detectLanes[i];
			//ƽ���ٶ�(km/h)
			double speed = cache.TotalTime == 0 ? 0 : (cache.TotalDistance * cache.MeterPerPixel / 1000.0) / (static_cast<double>(cache.TotalTime) / 3600000.0);
			//����ʱ��(sec)
			double headDistance = cache.Vehicles > 1 ? static_cast<double>(cache.TotalSpan) / static_cast<double>(cache.Vehicles - 1) / 1000.0 : 0;
			//��ͷ���(m)
			double headSpace = speed * 1000 * headDistance / 3600.0;
			//ʱ��ռ����(%)
			double timeOccupancy = static_cast<double>(cache.TotalInTime) / 60000.0 * 100;
			int trafficStatus = 0;
			if (speed > 40)
			{
				trafficStatus = static_cast<int>(TrafficStatus::Good);
			}
			else if (speed <= 40 && speed > 30)
			{
				trafficStatus = static_cast<int>(TrafficStatus::Normal);
			}
			else if (speed <= 30 && speed > 20)
			{
				trafficStatus = static_cast<int>(TrafficStatus::Warning);
			}
			else if (speed <= 20 && speed > 15)
			{
				trafficStatus = static_cast<int>(TrafficStatus::Warning);
			}
			else
			{
				trafficStatus = static_cast<int>(TrafficStatus::Dead);
			}
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

			JsonSerialization::SerializeValue(&laneJson, "averageSpeed", static_cast<int>(speed));
			JsonSerialization::SerializeValue(&laneJson, "headDistance", headDistance);
			JsonSerialization::SerializeValue(&laneJson, "headSpace", headSpace);
			JsonSerialization::SerializeValue(&laneJson, "timeOccupancy", static_cast<int>(timeOccupancy));
			JsonSerialization::SerializeValue(&laneJson, "trafficStatus", trafficStatus);
			JsonSerialization::AddClassItem(&flowLanesJson, laneJson);

			LogPool::Debug(LogEvent::Detect, "lane:", cache.LaneId, "vehicles:", cache.Cars + cache.Tricycles + cache.Buss + cache.Vans + cache.Trucks, "bikes:", cache.Bikes + cache.Motorcycles, "persons:", cache.Persons, speed, "km/h ", headDistance, "sec ", timeOccupancy, "%");

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
		//�������Ϊ�÷��ӽ�������ǰ֡�յ������ݽ��㵽��һ����
		DateTime currentTime(timeStamp);
		DateTime currentTimePoint(currentTime.Year(), currentTime.Month(), currentTime.Day(), currentTime.Hour(), currentTime.Minute(), 0);
		_currentMinuteTimeStamp = currentTimePoint.UtcTimeStamp();
		_lastFrameTimeStamp = _currentMinuteTimeStamp;
		_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
		if (!flowLanesJson.empty())
		{
			if (_mqtt != NULL)
			{
				_mqtt->Send(FlowTopic, flowLanesJson);
			}
		}
	}

	//���㵱ǰ֡
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
				//������³�����������ͳ�ͷʱ��
				//����=������
				//��ͷʱ��=���н��������ʱ����ƽ��ֵ
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
				//������Ѿ���¼�ĳ������ƽ���ٶ�
				//ƽ���ٶ�=�ܾ���/��ʱ��
				//�ܾ���=���μ�⵽�ĵ�ľ���*ÿ�����ش��������
				//��ʱ��=���μ�⵽��ʱ���ʱ��
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

		//�����һ���г�������Ϊ����μ��Ϊֹ���г�
		//ʱ��ռ����=��ʱ��/һ����
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
	if (!ioLanesJson.empty())
	{
		if (_mqtt != NULL)
		{
			_mqtt->Send(IOTopic, ioLanesJson);
		}
	}
	DrawDetect(*detectItems, iveBuffer, frameIndex);
}

void FlowDetector::HandleRecognize(const RecognItem& recognItem, const unsigned char* iveBuffer, const string& recognJson)
{
	if (_debug)
	{
		return;
	}
	lock_guard<mutex> recognLock(_recognLaneMutex);
	long long timeStamp = DateTime::UtcNowTimeStamp();
	JsonDeserialization jd(recognJson);	
	if (recognItem.Type == static_cast<int>(DetectType::Pedestrain))
	{
		int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Type"));
		if (pedestrainType != 0)
		{
			for (unsigned int i = 0; i < _recognLanes.size(); ++i)
			{
				if (_recognLanes[i].Region.Contains(recognItem.Region.HitPoint()))
				{
					int sex = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Sex:TopList:0:Code"));
					int age = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Age:TopList:0:Code"));
					int upperColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
					//pedestrain.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Feature:Feature"));
					string videoStructJson;
					JsonSerialization::SerializeValue(&videoStructJson, "channelUrl", _recognChannelUrl);
					JsonSerialization::SerializeValue(&videoStructJson, "laneId", _recognLanes[i].LaneId);
					JsonSerialization::SerializeValue(&videoStructJson, "timeStamp", timeStamp);
					//JsonSerialization::SerializeValue(&videoStructJson, "feature", pedestrain.Feature);
					IveToBgr(iveBuffer, recognItem.Width, recognItem.Height, _bgrBuffer);
					int jpgSize = BgrToJpg(_bgrBuffer, recognItem.Width, recognItem.Height, &_jpgBuffer);
					string image;
					JpgToBase64(&image, _jpgBuffer, jpgSize);
					JsonSerialization::SerializeValue(&videoStructJson, "image", image);
					JsonSerialization::SerializeValue(&videoStructJson, "videoStructType", (int)VideoStructType::Pedestrain);
					JsonSerialization::SerializeValue(&videoStructJson, "sex", sex);
					JsonSerialization::SerializeValue(&videoStructJson, "age", age);
					JsonSerialization::SerializeValue(&videoStructJson, "upperColor", upperColor);
					if (_mqtt != NULL)
					{
						_mqtt->Send(VideoStructTopic, videoStructJson);
					}
					LogPool::Debug(LogEvent::Detect, "lane:", _recognLanes[i].LaneId, "pedestrain");
					return;
				}
			}
		}
	}
	else if (recognItem.Type == static_cast<int>(DetectType::Bike)
		|| recognItem.Type == static_cast<int>(DetectType::Motobike))
	{
		int bikeType = jd.Get<int>(StringEx::Combine("ImageResults:0:Bikes:0:Type"));
		if (bikeType != 0)
		{
			for (unsigned int i = 0; i < _recognLanes.size(); ++i)
			{
				if (_recognLanes[i].Region.Contains(recognItem.Region.HitPoint()))
				{
					//bike.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Recognize:Feature:Feature"));
					string videoStructJson;
					JsonSerialization::SerializeValue(&videoStructJson, "channelUrl", _recognChannelUrl);
					JsonSerialization::SerializeValue(&videoStructJson, "laneId", _recognLanes[i].LaneId);
					JsonSerialization::SerializeValue(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::SerializeValue(&videoStructJson, "videoStructType", (int)VideoStructType::Bike);
					//JsonSerialization::SerializeValue(&videoStructJson, "feature", bike.Feature);
					IveToBgr(iveBuffer, recognItem.Width, recognItem.Height, _bgrBuffer);
					int jpgSize = BgrToJpg(_bgrBuffer, recognItem.Width, recognItem.Height, &_jpgBuffer);
					string image;
					JpgToBase64(&image, _jpgBuffer, jpgSize);
					JsonSerialization::SerializeValue(&videoStructJson, "image", image);
					JsonSerialization::SerializeValue(&videoStructJson, "bikeType", bikeType);
					if (_mqtt != NULL)
					{
						_mqtt->Send(VideoStructTopic, videoStructJson);
					}
					LogPool::Debug(LogEvent::Detect, "lane:", _recognLanes[i].LaneId, "bike:", bikeType);
					return;
				}
			}
		}
	}
	else
	{
		int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Type"));
		if (vehicleType != 0)
		{
			for (unsigned int i = 0; i < _recognLanes.size(); ++i)
			{
				if (_recognLanes[i].Region.Contains(recognItem.Region.HitPoint()))
				{
					int carType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Type:TopList:0:Code"));
					int carColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Color:TopList:0:Code"));
					string carBrand = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Brand:TopList:0:Name"));
					int plateType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Type"));
					string plateNumber = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Licence"));
					//vehicle.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Feature:Feature"));
					string videoStructJson;
					JsonSerialization::SerializeValue(&videoStructJson, "channelUrl", _recognChannelUrl);
					JsonSerialization::SerializeValue(&videoStructJson, "laneId", _recognLanes[i].LaneId);
					JsonSerialization::SerializeValue(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::SerializeValue(&videoStructJson, "videoStructType", (int)VideoStructType::Vehicle);
					//JsonSerialization::SerializeValue(&videoStructJson, "feature", vehicle.Feature);
					IveToBgr(iveBuffer, recognItem.Width, recognItem.Height, _bgrBuffer);
					int jpgSize = BgrToJpg(_bgrBuffer, recognItem.Width, recognItem.Height, &_jpgBuffer);
					string image;
					JpgToBase64(&image, _jpgBuffer, jpgSize);
					JsonSerialization::SerializeValue(&videoStructJson, "image", image);
					JsonSerialization::SerializeValue(&videoStructJson, "carType", carType);
					JsonSerialization::SerializeValue(&videoStructJson, "carColor", carColor);
					JsonSerialization::SerializeValue(&videoStructJson, "carBrand", carBrand);
					JsonSerialization::SerializeValue(&videoStructJson, "plateType", plateType);
					JsonSerialization::SerializeValue(&videoStructJson, "plateNumber", plateNumber);
					if (_mqtt != NULL)
					{
						_mqtt->Send(VideoStructTopic, videoStructJson);
					}
					LogPool::Debug(LogEvent::Detect, "lane:", _recognLanes[i].LaneId, "vehicle:", carType);
					return;
				}
			}
		}
	}
}

void FlowDetector::DrawDetect(const map<string, DetectItem>& detectItems, const unsigned char* iveBuffer, int frameIndex)
{
	if (!_debug)
	{
		return;
	}
	IveToBgr(iveBuffer,_width,_height,_bgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _bgrBuffer);
	for (unsigned int i = 0; i < _detectLanes.size(); ++i)
	{
		FlowLaneCache& cache = _detectLanes[i];
		DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
	}
	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		cv::Point point(it->second.Region.HitPoint().X, it->second.Region.HitPoint().Y);
		cv::Scalar scalar;
		//��ɫ�³�
		if (it->second.Status == DetectStatus::New)
		{
			scalar = cv::Scalar(0, 255, 0);
		}
		//��ɫ������
		else if (it->second.Status == DetectStatus::In)
		{
			scalar = cv::Scalar(0, 255, 255);
		}
		//��ɫ��������
		else
		{
			scalar = cv::Scalar(255, 0, 0);
		}
		cv::circle(image, point, 10 , scalar, -1);
	}

	int jpgSize = BgrToJpg(image.data, _width, _height,&_jpgBuffer);
	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, frameIndex);
}

