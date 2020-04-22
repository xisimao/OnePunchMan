#include "ChannelDetector.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;
using namespace TerribleTornado;

const string ChannelDetector::IOTopic("IO");
const string ChannelDetector::VideoStructTopic("VideoStruct");

ChannelDetector::ChannelDetector(int width, int height, MqttChannel* mqtt)
	:_mqtt(mqtt)
{

}

void ChannelDetector::UpdateChannel(const FlowChannel& channel)
{
	lock_guard<mutex> lck(_laneMutex);
	for (vector<LaneDetector*>::iterator it = _lanes.begin();it!=_lanes.end(); ++it)
	{
		delete (*it);
	}
	_lanes.clear();
	for (vector<Lane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
	{
		_lanes.push_back(new LaneDetector(lit->LaneId, *lit));
	}
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

}

void ChannelDetector::GetRecognGuids(vector<string>* guids, const JsonDeserialization& jd, const string& key)
{
	int itemIndex = 0;
	while (true)
	{
		string id = jd.Get<string>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":GUID"));
		if (id.empty())
		{
			break;
		}
		guids->push_back(id);
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
			items->insert(pair<string, DetectItem>(id, DetectItem(Saitama::Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]), type)));
		}
		itemIndex += 1;
	}
}

void ChannelDetector::CollectFlow(string* flowJson, long long timeStamp)
{
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
	{
		LaneDetector* detector = _lanes[laneIndex];
		LaneItem item = detector->Collect(timeStamp);

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
	lck.unlock();
}

vector<string> ChannelDetector::HandleDetect(const string& detectJson, long long timeStamp)
{
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

	vector<string> recognGuids;
	GetRecognGuids(&recognGuids, detectJd, "Vehicles");
	GetRecognGuids(&recognGuids, detectJd, "Bikes");
	GetRecognGuids(&recognGuids, detectJd, "Pedestrains");
	return recognGuids;
}

void ChannelDetector::HandleRecognize(const string& recognJson)
{
	long long timeStamp = DateTime::UtcTimeStamp();
	JsonDeserialization jd(recognJson);
	
	int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Type"));
	if (vehicleType != 0)
	{
		vector<int> vehicleRect = jd.GetArray<int>(StringEx::Combine("l1_result:0:Detect:Body:Rect"));
		if (vehicleRect.size() >= 4)
		{
			DetectItem item(Saitama::Rectangle(Point(vehicleRect[0], vehicleRect[1]), vehicleRect[2], vehicleRect[3]));
			lock_guard<mutex> lck(_laneMutex);
			for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
			{
				if (_lanes[laneIndex]->Contains(item))
				{
					VideoVehicle vehicle;
					vehicle.CarType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Type:TopList:0:Code"));
					vehicle.CarColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Color:TopList:0:Code"));
					vehicle.CarBrand = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Brand:TopList:0:Name"));
					vehicle.PlateType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Type"));
					vehicle.PlateNumber = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Licence"));
			/*		vehicle.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Feature:Feature"));
					vehicle.Image = images[imageIndex];*/

					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", vehicle.LaneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", vehicle.VideoStructType);
					JsonSerialization::Serialize(&videoStructJson, "feature", vehicle.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", vehicle.Image);
					JsonSerialization::Serialize(&videoStructJson, "carType", vehicle.CarType);
					JsonSerialization::Serialize(&videoStructJson, "carColor", vehicle.CarColor);
					JsonSerialization::Serialize(&videoStructJson, "carBrand", vehicle.CarBrand);
					JsonSerialization::Serialize(&videoStructJson, "plateType", vehicle.PlateType);
					JsonSerialization::Serialize(&videoStructJson, "plateNumber", vehicle.PlateNumber);
					_mqtt->Send(VideoStructTopic, videoStructJson);
					LogPool::Debug(LogEvent::Detect, "lane:", vehicle.LaneId, "vehicle:", vehicle.CarType);
					return;
				}
			}
		}
	}

	int bikeType = jd.Get<int>(StringEx::Combine("ImageResults:0:Bikes:0:Type"));
	if (bikeType != 0)
	{
		vector<int> bikeRect = jd.GetArray<int>(StringEx::Combine("l1_result:0:Detect:Body:Rect"));
		if (bikeRect.size() >= 4)
		{
			DetectItem item(Saitama::Rectangle(Point(bikeRect[0], bikeRect[1]), bikeRect[2], bikeRect[3]));
			lock_guard<mutex> lck(_laneMutex);
			for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
			{
				if (_lanes[laneIndex]->Contains(item))
				{
					VideoBike bike;
					bike.BikeType = bikeType;
	/*				bike.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Recognize:Feature:Feature"));
					bike.Image = images[imageIndex];*/

					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", bike.LaneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", bike.VideoStructType);
					JsonSerialization::Serialize(&videoStructJson, "feature", bike.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", bike.Image);
					JsonSerialization::Serialize(&videoStructJson, "bikeType", bike.BikeType);
					_mqtt->Send(VideoStructTopic, videoStructJson);
					LogPool::Debug(LogEvent::Detect, "lane:", bike.LaneId, "bike:", bike.BikeType);
					return;
				}
			}
		}
	}

	int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Type"));
	if (pedestrainType != 0)
	{
		vector<int> pedestrainRect = jd.GetArray<int>(StringEx::Combine("l1_result:0:Detect:Body:Rect"));
		if (pedestrainRect.size() >= 4)
		{
			DetectItem item(Saitama::Rectangle(Point(pedestrainRect[0], pedestrainRect[1]), pedestrainRect[2], pedestrainRect[3]));
			lock_guard<mutex> lck(_laneMutex);
			for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
			{
				if (_lanes[laneIndex]->Contains(item))
				{
					VideoPedestrain pedestrain;
					pedestrain.Sex = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Sex:TopList:0:Code"));
					pedestrain.Age = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Age:TopList:0:Code"));
					pedestrain.UpperColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
					/*pedestrain.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Feature:Feature"));
					pedestrain.Image = images[imageIndex];*/
					string videoStructJson;
					JsonSerialization::Serialize(&videoStructJson, "channelUrl", _channelUrl);
					JsonSerialization::Serialize(&videoStructJson, "laneId", pedestrain.LaneId);
					JsonSerialization::Serialize(&videoStructJson, "timeStamp", timeStamp);
					JsonSerialization::Serialize(&videoStructJson, "feature", pedestrain.Feature);
					JsonSerialization::Serialize(&videoStructJson, "image", pedestrain.Image);
					JsonSerialization::Serialize(&videoStructJson, "videoStructType", pedestrain.VideoStructType);
					JsonSerialization::Serialize(&videoStructJson, "sex", pedestrain.Sex);
					JsonSerialization::Serialize(&videoStructJson, "age", pedestrain.Age);
					JsonSerialization::Serialize(&videoStructJson, "upperColor", pedestrain.UpperColor);
					_mqtt->Send(VideoStructTopic, videoStructJson);
					LogPool::Debug(LogEvent::Detect, "lane:", pedestrain.LaneId, "pedestrain");
					return;
				}
			}
		}
	}
}

