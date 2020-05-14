#include "FlowDetector.h"

using namespace std;
using namespace OnePunchMan;

const string FlowDetector::IOTopic("IO");
const string FlowDetector::VideoStructTopic("VideoStruct");

FlowDetector::FlowDetector(int width, int height, MqttChannel* mqtt,bool debug)
	:TrafficDetector(width,height,mqtt,debug), _lastTimeStamp(0)
	
{
}

void FlowDetector::UpdateChannel(const FlowChannel& channel)
{
	unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
	if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
	{
		_lanes.clear();
		string regionsParam;
		regionsParam.append("[");
		_lanesInited = true;
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
				_lanes.push_back(cache);
				regionsParam.append(lit->Region);
				regionsParam.append(",");
			}
		}
		_lanesInited = !_lanes.empty() && _lanes.size() == channel.Lanes.size();
		if (!_lanesInited)
		{
			LogPool::Warning("lane init failed", channel.ChannelIndex);
		}
		if (regionsParam.size() == 1)
		{
			regionsParam.append("]");
		}
		else
		{
			regionsParam[regionsParam.size() - 1] = ']';
		}

		_param = StringEx::Combine("{\"Detect\":{\"DetectRegion\":", regionsParam, ",\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}");
		_setParam = false;
		_channelIndex = channel.ChannelIndex;
		_channelUrl = channel.ChannelUrl;
	}
	else
	{
		LogPool::Error(LogEvent::Thread, "update channel lock timeout");
	}	
}

void FlowDetector::ClearChannel()
{
	unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
	if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
	{
		_lanes.clear();
		_channelUrl = string();
		_channelIndex = 0;
		_lanesInited = false;
	}
	else
	{
		LogPool::Error(LogEvent::Thread, "clear channel lock timeout");
	}
}

void FlowDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, string* param, const unsigned char* iveBuffer, int packetIndex, int frameSpan)
{
	if (_debug)
	{
		timeStamp = packetIndex * frameSpan;
	}
	string lanesJson;
	string channelUrl;
	unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
	if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
	{
		channelUrl = _channelUrl;
		if (!_setParam)
		{
			param->assign(_param);
			_setParam = true;
		}
		for (unsigned int i = 0; i < _lanes.size(); ++i)
		{
			FlowLaneCache& cache = _lanes[i];
			cache.CurrentItems()->clear();

			bool ioStatus = false;
			DetectType type = DetectType::None;
			for (map<string, DetectItem>::iterator it = detectItems->begin(); it != detectItems->end(); ++it)
			{
				if (it->second.Status != DetectStatus::Out)
				{
					continue;
				}
				if (cache.Region.Contains(it->second.Region.HitPoint()))
				{
					ioStatus = true;
					type = it->second.Type;
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
						cache.TotalTime += timeStamp - _lastTimeStamp;
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
				cache.TotalInTime += timeStamp - _lastTimeStamp;
			}

			cache.SwitchFlag();

			if (ioStatus != cache.IoStatus)
			{
				cache.IoStatus = ioStatus;
				string laneJson;
				JsonSerialization::Serialize(&laneJson, "laneId", cache.LaneId);
				JsonSerialization::Serialize(&laneJson, "status", (int)ioStatus);
				JsonSerialization::Serialize(&laneJson, "type", (int)type);
				JsonSerialization::SerializeItem(&lanesJson, laneJson);
				LogPool::Debug(LogEvent::Detect, "lane:", cache.LaneId, "io:", ioStatus);
			}
		}
		_lastTimeStamp = timeStamp;

		lck.unlock();
	}
	else
	{
		LogPool::Error(LogEvent::Thread, "flow detect lock timeout");
	}
	
	if (!lanesJson.empty())
	{
		string channelJson;
		JsonSerialization::SerializeJson(&channelJson, "lanes", lanesJson);
		JsonSerialization::Serialize(&channelJson, "channelUrl", channelUrl);
		JsonSerialization::Serialize(&channelJson, "timeStamp", timeStamp);
		if (_mqtt != NULL)
		{
			_mqtt->Send(IOTopic, channelJson);
		}
	}
	DrawDetect(*detectItems, iveBuffer, packetIndex);
}

void FlowDetector::HandleRecognize(const RecognItem& recognItem, const unsigned char* iveBuffer, const string& recognJson)
{
	long long timeStamp = DateTime::UtcNowTimeStamp();
	IveToBgr(iveBuffer, recognItem.Width, recognItem.Height, _bgrBuffer);
	int jpgSize=BgrToJpg(_bgrBuffer, recognItem.Width, recognItem.Height,&_jpgBuffer);
	string image;
	JpgToBase64(&image, _jpgBuffer, jpgSize);
	JsonDeserialization jd(recognJson);	
	if (recognItem.Type == static_cast<int>(DetectType::Pedestrain))
	{
		int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Type"));
		if (pedestrainType != 0)
		{
			unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
			if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
			{
				for (unsigned int i = 0; i < _lanes.size(); ++i)
				{
					if (_lanes[i].Region.Contains(recognItem.Region.HitPoint()))
					{
						string channelUrl = _channelUrl;
						string laneId = _lanes[i].LaneId;
						lck.unlock();
						int sex = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Sex:TopList:0:Code"));
						int age = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Age:TopList:0:Code"));
						int upperColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
						//pedestrain.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Feature:Feature"));
						string videoStructJson;
						JsonSerialization::Serialize(&videoStructJson, "channelUrl", channelUrl);
						JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
						JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
						//JsonSerialization::Serialize(&videoStructJson, "feature", pedestrain.Feature);
						JsonSerialization::Serialize(&videoStructJson, "image", image);
						JsonSerialization::Serialize(&videoStructJson, "videoStructType", (int)VideoStructType::Pedestrain);
						JsonSerialization::Serialize(&videoStructJson, "sex", sex);
						JsonSerialization::Serialize(&videoStructJson, "age", age);
						JsonSerialization::Serialize(&videoStructJson, "upperColor", upperColor);
						if (_mqtt != NULL)
						{
							_mqtt->Send(VideoStructTopic, videoStructJson);
						}
						LogPool::Debug(LogEvent::Detect, "lane:", laneId, "pedestrain");
						return;
					}
				}
			}
			else
			{
				LogPool::Error(LogEvent::Thread, "flow recogn lock timeout");
			}
		}
	}
	else if (recognItem.Type == static_cast<int>(DetectType::Bike)
		|| recognItem.Type == static_cast<int>(DetectType::Motobike))
	{
		int bikeType = jd.Get<int>(StringEx::Combine("ImageResults:0:Bikes:0:Type"));
		if (bikeType != 0)
		{
			unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
			if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
			{
				for (unsigned int i = 0; i < _lanes.size(); ++i)
				{
					if (_lanes[i].Region.Contains(recognItem.Region.HitPoint()))
					{
						string laneId = _lanes[i].LaneId;
						string channelUrl = _channelUrl;
						lck.unlock();
						//bike.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Recognize:Feature:Feature"));
						string videoStructJson;
						JsonSerialization::Serialize(&videoStructJson, "channelUrl", channelUrl);
						JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
						JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
						JsonSerialization::Serialize(&videoStructJson, "videoStructType", (int)VideoStructType::Bike);
						//JsonSerialization::Serialize(&videoStructJson, "feature", bike.Feature);
						JsonSerialization::Serialize(&videoStructJson, "image", image);
						JsonSerialization::Serialize(&videoStructJson, "bikeType", bikeType);
						if (_mqtt != NULL)
						{
							_mqtt->Send(VideoStructTopic, videoStructJson);
						}
						LogPool::Debug(LogEvent::Detect, "lane:", laneId, "bike:", bikeType);
						return;
					}
				}
			}
			else
			{
				LogPool::Error(LogEvent::Thread, "flow recogn lock timeout");
			}		
		}
	}
	else
	{
		int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Type"));
		if (vehicleType != 0)
		{
			unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
			if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
			{
				for (unsigned int i = 0; i < _lanes.size(); ++i)
				{
					if (_lanes[i].Region.Contains(recognItem.Region.HitPoint()))
					{
						string laneId = _lanes[i].LaneId;
						string channelUrl = _channelUrl;
						lck.unlock();
						int carType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Type:TopList:0:Code"));
						int carColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Color:TopList:0:Code"));
						string carBrand = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Brand:TopList:0:Name"));
						int plateType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Type"));
						string plateNumber = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Licence"));
						//vehicle.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Feature:Feature"));
						string videoStructJson;
						JsonSerialization::Serialize(&videoStructJson, "channelUrl", channelUrl);
						JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
						JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
						JsonSerialization::Serialize(&videoStructJson, "videoStructType", (int)VideoStructType::Vehicle);
						//JsonSerialization::Serialize(&videoStructJson, "feature", vehicle.Feature);
						JsonSerialization::Serialize(&videoStructJson, "image", image);
						JsonSerialization::Serialize(&videoStructJson, "carType", carType);
						JsonSerialization::Serialize(&videoStructJson, "carColor", carColor);
						JsonSerialization::Serialize(&videoStructJson, "carBrand", carBrand);
						JsonSerialization::Serialize(&videoStructJson, "plateType", plateType);
						JsonSerialization::Serialize(&videoStructJson, "plateNumber", plateNumber);
						if (_mqtt != NULL)
						{
							_mqtt->Send(VideoStructTopic, videoStructJson);
						}
						LogPool::Debug(LogEvent::Detect, "lane:", laneId, "vehicle:", carType);
						return;
					}
				}
			}
			else
			{
				LogPool::Error(LogEvent::Thread, "flow recogn lock timeout");
			}
		}
	}
}

void FlowDetector::CollectFlow(string* flowJson, long long timeStamp)
{
	unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
	if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
	{
		for (unsigned int i = 0; i < _lanes.size(); ++i)
		{
			FlowLaneCache& cache = _lanes[i];

			double speed = cache.TotalTime == 0 ? 0 : (cache.TotalDistance * cache.MeterPerPixel / 1000.0) / (static_cast<double>(cache.TotalTime) / 3600000.0);
			double headDistance = cache.Vehicles > 1 ? static_cast<double>(cache.TotalSpan) / static_cast<double>(cache.Vehicles - 1) / 1000.0 : 0;
			double headSpace = speed * 1000 * headDistance / 3600.0;
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
			JsonSerialization::Serialize(&laneJson, "channelUrl", _channelUrl);
			JsonSerialization::Serialize(&laneJson, "laneId", cache.LaneId);
			JsonSerialization::Serialize(&laneJson, "timeStamp", timeStamp);
			JsonSerialization::Serialize(&laneJson, "persons", cache.Persons);
			JsonSerialization::Serialize(&laneJson, "bikes", cache.Bikes);
			JsonSerialization::Serialize(&laneJson, "motorcycles", cache.Motorcycles);
			JsonSerialization::Serialize(&laneJson, "cars", cache.Cars);
			JsonSerialization::Serialize(&laneJson, "tricycles", cache.Tricycles);
			JsonSerialization::Serialize(&laneJson, "buss", cache.Buss);
			JsonSerialization::Serialize(&laneJson, "vans", cache.Vans);
			JsonSerialization::Serialize(&laneJson, "trucks", cache.Trucks);

			JsonSerialization::Serialize(&laneJson, "averageSpeed", static_cast<int>(speed));
			JsonSerialization::Serialize(&laneJson, "headDistance", headDistance);
			JsonSerialization::Serialize(&laneJson, "headSpace", headSpace);
			JsonSerialization::Serialize(&laneJson, "timeOccupancy", static_cast<int>(timeOccupancy));
			JsonSerialization::Serialize(&laneJson, "trafficStatus", trafficStatus);
			JsonSerialization::SerializeItem(flowJson, laneJson);

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
		_lastTimeStamp = timeStamp;
	}
	else
	{
		LogPool::Error(LogEvent::Thread, "flow collect lock timeout");
	}
}

void FlowDetector::DrawDetect(const map<string, DetectItem>& detectItems, const unsigned char* iveBuffer, int packetIndex)
{
	if (!_debug)
	{
		return;
	}
	IveToBgr(iveBuffer,_width,_height,_debugBgrBuffer);
	cv::Mat image(_height, _width, CV_8UC3, _debugBgrBuffer);
	unique_lock<timed_mutex> lck(_laneMutex, std::defer_lock);
	if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
	{
		for (unsigned int i = 0; i < _lanes.size(); ++i)
		{
			FlowLaneCache& cache = _lanes[i];
			DrawPolygon(&image, cache.Region, cv::Scalar(0, 0, 255));
		}
		lck.unlock();
	}
	else
	{
		LogPool::Error(LogEvent::Thread, "flow debug lock timeout");
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
	_jpgHandler.HandleFrame(_jpgBuffer, jpgSize, packetIndex);
}

