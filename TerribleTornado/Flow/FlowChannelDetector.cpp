#include "FlowChannelDetector.h"

using namespace std;
using namespace OnePunchMan;

const string FlowChannelDetector::IOTopic("IO");
const string FlowChannelDetector::VideoStructTopic("VideoStruct");

FlowChannelDetector::FlowChannelDetector(int width, int height, MqttChannel* mqtt,bool debug)
	:ChannelDetector(width,height,mqtt)
	,_lanesInited(false),_debug(debug), _debugBgrBuffer(new unsigned char[width * height * 3]), _bgrHandler(), _jpgHandler()
{
}

FlowChannelDetector::~FlowChannelDetector()
{
	delete[] _debugBgrBuffer;
}

bool FlowChannelDetector::LanesInited() const
{
	return _lanesInited;
}

void FlowChannelDetector::UpdateChannel(const FlowChannel& channel)
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<LaneDetector*>::iterator it = _lanes.begin();it!=_lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
	string regionsParam;
	regionsParam.append("[");
	_lanesInited = true;
	for (vector<FlowLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
	{
		regionsParam.append(lit->Region);
		regionsParam.append(",");
		LaneDetector* laneDetector = new LaneDetector(*lit);
		if (!laneDetector->Inited())
		{
			_lanesInited = false;
		}	
		_lanes.push_back(laneDetector);
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

void FlowChannelDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<LaneDetector*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
	_channelUrl = string();
	_channelIndex = 0;
	_lanesInited = false;
}

void FlowChannelDetector::CollectFlow(string* flowJson, long long timeStamp)
{
	lock_guard<mutex> lck(_laneMutex);
	for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
	{
		LaneDetector* detector = _lanes[laneIndex];
		FlowResult item = detector->Collect(timeStamp);

		string laneJson;
		JsonSerialization::Serialize(&laneJson, "channelUrl", _channelUrl);
		JsonSerialization::Serialize(&laneJson, "laneId", item.LaneId);
		JsonSerialization::Serialize(&laneJson, "timeStamp", timeStamp);
		JsonSerialization::Serialize(&laneJson, "persons", item.Persons);
		JsonSerialization::Serialize(&laneJson, "bikes", item.Bikes);
		JsonSerialization::Serialize(&laneJson, "motorcycles", item.Motorcycles);
		JsonSerialization::Serialize(&laneJson, "cars", item.Cars);
		JsonSerialization::Serialize(&laneJson, "tricycles", item.Tricycles);
		JsonSerialization::Serialize(&laneJson, "buss", item.Buss);
		JsonSerialization::Serialize(&laneJson, "vans", item.Vans);
		JsonSerialization::Serialize(&laneJson, "trucks", item.Trucks);

		JsonSerialization::Serialize(&laneJson, "averageSpeed", static_cast<int>(item.Speed));
		JsonSerialization::Serialize(&laneJson, "headDistance", item.HeadDistance);
		JsonSerialization::Serialize(&laneJson, "headSpace", item.HeadSpace);
		JsonSerialization::Serialize(&laneJson, "timeOccupancy", static_cast<int>(item.TimeOccupancy));
		JsonSerialization::Serialize(&laneJson, "trafficStatus", item.TrafficStatus);
		JsonSerialization::SerializeItem(flowJson, laneJson);
	}
}

void FlowChannelDetector::HandleDetectCore(std::map<std::string, DetectItem> detectItems, long long timeStamp, const unsigned char* iveBuffer, long long packetIndex)
{
	string lanesJson;
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
	{
		IOResult item = _lanes[laneIndex]->Detect(&detectItems, timeStamp);
		if (item.Changed)
		{
			string laneJson;
			JsonSerialization::Serialize(&laneJson, "laneId", item.LaneId);
			JsonSerialization::Serialize(&laneJson, "status", (int)item.Status);
			JsonSerialization::Serialize(&laneJson, "type", (int)item.Type);
			JsonSerialization::SerializeItem(&lanesJson, laneJson);
		}
	}
	lck.unlock();
	if (!lanesJson.empty())
	{
		string channelJson;
		JsonSerialization::SerializeJson(&channelJson, "lanes", lanesJson);
		JsonSerialization::Serialize(&channelJson, "channelUrl", _channelUrl);
		JsonSerialization::Serialize(&channelJson, "timeStamp", timeStamp);
		if (_mqtt != NULL)
		{
			_mqtt->Send(IOTopic, channelJson);
		}
	}
	DrawDetect(detectItems, iveBuffer, packetIndex);
}

void FlowChannelDetector::HandleRecognize(const RecognItem& recognItem, const unsigned char* iveBuffer, const string& recognJson)
{
	long long timeStamp = DateTime::UtcNowTimeStamp();

	tjhandle handle = tjInitCompress();
	unsigned char* jpgBuffer = NULL;
	unsigned long jpgSize;
	string jpgBase64("data:image/jpg;base64,");
	IveToBgr(iveBuffer, recognItem.Width, recognItem.Height,_bgrBuffer);
	
	if (tjCompress2(handle, _bgrBuffer, recognItem.Width, 0, recognItem.Height, TJPF_BGR, &jpgBuffer, &jpgSize, TJSAMP_422,10, 0) == 0)
	{
		StringEx::ToBase64String(jpgBuffer, jpgSize, &jpgBase64);
	}
	//long long l = DateTime::UtcNowTimeStamp();
	//_bgrHandler.HandleFrame(_bgrBuffer, recognItem.Width, recognItem.Height, l);
	//_jpgHandler.HandleFrame(jpgBuffer, jpgSize, l);
	tjDestroy(handle);
	tjFree(jpgBuffer);
	JsonDeserialization jd(recognJson);	
	if (recognItem.Type == static_cast<int>(DetectType::Pedestrain))
	{
		int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Type"));
		if (pedestrainType != 0)
		{
			unique_lock<mutex> lck(_laneMutex);
			for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
			{
				string laneId = _lanes[laneIndex]->Recogn(recognItem);
				if (!laneId.empty())
				{
					lck.unlock();
					VideoPedestrain pedestrain;
					pedestrain.Sex = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Sex:TopList:0:Code"));
					pedestrain.Age = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Age:TopList:0:Code"));
					pedestrain.UpperColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
					//pedestrain.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Feature:Feature"));
					
					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					//JsonSerialization::Serialize(&videoStructJson, "feature", pedestrain.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", jpgBase64);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", pedestrain.VideoStructType);
					JsonSerialization::Serialize(&videoStructJson, "sex", pedestrain.Sex);
					JsonSerialization::Serialize(&videoStructJson, "age", pedestrain.Age);
					JsonSerialization::Serialize(&videoStructJson, "upperColor", pedestrain.UpperColor);
					if (_mqtt != NULL)
					{
						_mqtt->Send(VideoStructTopic, videoStructJson);
					}
					LogPool::Debug(LogEvent::Detect, "lane:", laneId, "pedestrain");
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
			unique_lock<mutex> lck(_laneMutex);
			for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
			{
				string laneId = _lanes[laneIndex]->Recogn(recognItem);
				if (!laneId.empty())
				{
					lck.unlock();
					VideoBike bike;
					bike.BikeType = bikeType;
					//bike.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Recognize:Feature:Feature"));
					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", bike.VideoStructType);
					//JsonSerialization::Serialize(&videoStructJson, "feature", bike.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", jpgBase64);
					JsonSerialization::Serialize(&videoStructJson, "bikeType", bike.BikeType);				
					if (_mqtt != NULL)
					{
						_mqtt->Send(VideoStructTopic, videoStructJson);
					}
					LogPool::Debug(LogEvent::Detect, "lane:", laneId, "bike:", bike.BikeType);
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
			unique_lock<mutex> lck(_laneMutex);
			for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
			{
				string laneId = _lanes[laneIndex]->Recogn(recognItem);
				if (!laneId.empty())
				{
					lck.unlock();
					VideoVehicle vehicle;
					vehicle.CarType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Type:TopList:0:Code"));
					vehicle.CarColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Color:TopList:0:Code"));
					vehicle.CarBrand = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Brand:TopList:0:Name"));
					vehicle.PlateType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Type"));
					vehicle.PlateNumber = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Licence"));
					//vehicle.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Feature:Feature"));
					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", vehicle.VideoStructType);
					//JsonSerialization::Serialize(&videoStructJson, "feature", vehicle.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", jpgBase64);
					JsonSerialization::Serialize(&videoStructJson, "carType", vehicle.CarType);
					JsonSerialization::Serialize(&videoStructJson, "carColor", vehicle.CarColor);
					JsonSerialization::Serialize(&videoStructJson, "carBrand", vehicle.CarBrand);
					JsonSerialization::Serialize(&videoStructJson, "plateType", vehicle.PlateType);
					JsonSerialization::Serialize(&videoStructJson, "plateNumber", vehicle.PlateNumber);
					if (_mqtt != NULL)
					{
						_mqtt->Send(VideoStructTopic, videoStructJson);
					}
					LogPool::Debug(LogEvent::Detect, "lane:", laneId, "vehicle:", vehicle.CarType);
					return;
				}
			}
		}
	}
}

void FlowChannelDetector::DrawDetect(const map<string, DetectItem>& detectItems, const unsigned char* iveBuffer, long long packetIndex)
{
	if (!_debug)
	{
		return;
	}
	IveToBgr(iveBuffer,_width,_height,_debugBgrBuffer);
	//_bgrHandler.HandleFrame(_debugBgrBuffer, _width, _height, packetIndex);
	cv::Mat image(_height, _width, CV_8UC3, _debugBgrBuffer);

	vector<vector<cv::Point>> lanesPoints;
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int i = 0; i < _lanes.size(); ++i)
	{
		vector<cv::Point> points;
		for (unsigned int j = 0; j < _lanes[i]->Region().Points().size();++j)
		{
			points.push_back(cv::Point(_lanes[i]->Region().Points()[j].X, _lanes[i]->Region().Points()[j].Y));
		}
		lanesPoints.push_back(points);
	}
	lck.unlock();
	cv::polylines(image, lanesPoints, true, cv::Scalar(0, 0, 255), 3);
	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		cv::Point point(it->second.Region.HitPoint().X, it->second.Region.HitPoint().Y);
		cv::putText(image, StringEx::ToString(it->second.Distance), point, cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 0), 5);
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
		cv::circle(image, point, 10 , scalar, -1);
	}
	LogPool::Debug("detect result", detectItems.size(), packetIndex);

	tjhandle handle = tjInitCompress();
	unsigned char* jpgBuffer = NULL;
	unsigned long jpgSize;
	tjCompress2(handle, image.data, _width, 0, _height, TJPF_BGR, &jpgBuffer, &jpgSize, TJSAMP_422,10, 0);
	tjDestroy(handle);
	_jpgHandler.HandleFrame(jpgBuffer, jpgSize, packetIndex);
	tjFree(jpgBuffer);
}

