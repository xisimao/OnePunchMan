#include "ChannelDetector.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;
using namespace TerribleTornado;

const string ChannelDetector::IOTopic("IO");
const string ChannelDetector::VideoStructTopic("VideoStruct");

ChannelDetector::ChannelDetector(int width, int height, MqttChannel* mqtt)
	:_channelIndex(0),_channelUrl(),_mqtt(mqtt),_param(),_setParam(true)
{

}

string ChannelDetector::ChannelUrl()
{
	return _channelUrl;
}

void ChannelDetector::UpdateChannel(const FlowChannel& channel)
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<LaneDetector*>::iterator it = _lanes.begin();it!=_lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
	string regionsParam;
	regionsParam.append("[");
	for (vector<Lane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
	{
		regionsParam.append(lit->Region);
		regionsParam.append(",");
		_lanes.push_back(new LaneDetector(lit->LaneId, *lit));
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

void ChannelDetector::ClearChannel()
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<LaneDetector*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
	_channelUrl = string();
}

void ChannelDetector::GetRecognItems(vector<RecognItem>* items, const JsonDeserialization& jd, const string& key)
{
	int itemIndex = 0;
	while (true)
	{
		string id = jd.Get<string>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":GUID"));
		if (id.empty())
		{
			break;
		}
		vector<int> rect = jd.GetArray<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Rect"));
		if (rect.size() >= 4)
		{
			RecognItem item;
			item.ChannelIndex = _channelIndex;
			item.Guid = id;
			item.Type= jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Type"));
			item.Width = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Width"));
			item.Height = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Height"));
			Saitama::Rectangle rectangle(Point(rect[0], rect[1]), rect[2], rect[3]);
			item.HitPoint = rectangle.HitPoint;
			items->push_back(item);
		}
		itemIndex += 1;
	}
}

void ChannelDetector::GetDetecItems(map<string, DetectItem>* items, const JsonDeserialization& jd, const string& key)
{
	int itemIndex = 0;
	while (true)
	{
		string id = jd.Get<string>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":GUID"));
		if (id.empty())
		{
			break;
		}
		int type = jd.Get<int>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":Type"));
		vector<int> rect = jd.GetArray<int>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":Detect:Body:Rect"));
		if (rect.size() >= 4)
		{
			items->insert(pair<string, DetectItem>(id, DetectItem(Saitama::Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]).HitPoint, type)));
		}
		itemIndex += 1;
	}
}

void ChannelDetector::CollectFlow(string* flowJson, long long timeStamp)
{
	lock_guard<mutex> lck(_laneMutex);
	for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
	{
		LaneDetector* detector = _lanes[laneIndex];
		FlowItem item = detector->Collect(timeStamp);

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

vector<RecognItem> ChannelDetector::HandleDetect(const string& detectJson, std::string* param)
{
	if (!_setParam)
	{
		param->assign(_param);
		_setParam = true;
	}

	long long timeStamp = DateTime::UtcTimeStamp();
	JsonDeserialization detectJd(detectJson);
	map<string, DetectItem> detectItems;
	GetDetecItems(&detectItems, detectJd, "Vehicles");
	GetDetecItems(&detectItems, detectJd, "Bikes");
	GetDetecItems(&detectItems, detectJd, "Pedestrains");
	string lanesJson;
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
	{
		IOItem item = _lanes[laneIndex]->Detect(detectItems, timeStamp);
		if (item.Status != IOStatus::UnChanged)
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
		_mqtt->Send(IOTopic, channelJson, true);
	}

	vector<RecognItem> items;
	GetRecognItems(&items, detectJd, "Vehicles");
	GetRecognItems(&items, detectJd, "Bikes");
	GetRecognItems(&items, detectJd, "Pedestrains");
	return items;
}

void ChannelDetector::HandleRecognize(const RecognItem& recognItem, uint8_t* bgrBuffer, const string& recognJson)
{
	long long timeStamp = DateTime::UtcTimeStamp();
	JsonDeserialization jd(recognJson);	
	if (recognItem.Type == static_cast<int>(DetectType::Pedestrain))
	{
		int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Type"));
		if (pedestrainType != 0)
		{
			DetectItem detectItem(recognItem.HitPoint);
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
					_handler.ToBase64String(bgrBuffer, recognItem.Width,recognItem.Height,&pedestrain.Image);
					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					//JsonSerialization::Serialize(&videoStructJson, "feature", pedestrain.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", pedestrain.Image);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", pedestrain.VideoStructType);
					JsonSerialization::Serialize(&videoStructJson, "sex", pedestrain.Sex);
					JsonSerialization::Serialize(&videoStructJson, "age", pedestrain.Age);
					JsonSerialization::Serialize(&videoStructJson, "upperColor", pedestrain.UpperColor);
					_mqtt->Send(VideoStructTopic, videoStructJson);
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
			DetectItem item(recognItem.HitPoint);
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
					_handler.ToBase64String(bgrBuffer, recognItem.Width,recognItem.Height, &bike.Image);
					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", bike.VideoStructType);
					//JsonSerialization::Serialize(&videoStructJson, "feature", bike.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", bike.Image);
					JsonSerialization::Serialize(&videoStructJson, "bikeType", bike.BikeType);
					_mqtt->Send(VideoStructTopic, videoStructJson);
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
			DetectItem detectItem(recognItem.HitPoint);
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
					_handler.ToBase64String(bgrBuffer, recognItem.Width ,recognItem.Height , &vehicle.Image);
					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", laneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", vehicle.VideoStructType);
					//JsonSerialization::Serialize(&videoStructJson, "feature", vehicle.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", vehicle.Image);
					JsonSerialization::Serialize(&videoStructJson, "carType", vehicle.CarType);
					JsonSerialization::Serialize(&videoStructJson, "carColor", vehicle.CarColor);
					JsonSerialization::Serialize(&videoStructJson, "carBrand", vehicle.CarBrand);
					JsonSerialization::Serialize(&videoStructJson, "plateType", vehicle.PlateType);
					JsonSerialization::Serialize(&videoStructJson, "plateNumber", vehicle.PlateNumber);
					_mqtt->Send(VideoStructTopic, videoStructJson);
					LogPool::Debug(LogEvent::Detect, "lane:", laneId, "vehicle:", vehicle.CarType);
					return;
				}
			}
		}
	}
}

