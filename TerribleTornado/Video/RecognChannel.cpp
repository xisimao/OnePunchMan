#include "RecognChannel.h"

using namespace std;
using namespace Saitama;
using namespace TerribleTornado;

const int RecognChannel::ItemCount = 4;
const int RecognChannel::MaxCacheCount = 20;
const int RecognChannel::SleepTime = 50;
const string RecognChannel::VideoStructTopic("VideoStruct");

RecognChannel::RecognChannel(int channelIndex, int bgrWidth, int bgrHeight, MqttChannel* mqtt,vector<LaneDetector*> lanes)
	:ThreadObject("recogn"), _inited(false),_channelIndex(channelIndex),_mqtt(mqtt),_lanes(lanes)
{
	_recognData.resize(3 * bgrWidth * bgrHeight);
	_recognDataValue.push_back(_recognData.data());
	_guidValues.resize(1);
	_recognParam = "{\"Detect\":{\"DetectRegion\":[],\"IsDet\":true,\"MaxCarWidth\":0,\"MinCarWidth\":0,\"Mode\":0,\"Threshold\":20,\"Version\":1001},\"Recognize\":{\"Person\":{\"IsRec\":true},\"Feature\":{\"IsRec\":true},\"Vehicle\":{\"Brand\":{\"IsRec\":true},\"Plate\":{\"IsRec\":true},\"Color\":{\"IsRec\":true},\"Marker\":{\"IsRec\":true},\"Sunroof\":{\"IsRec\":true},\"SpareTire\":{\"IsRec\":true},\"Slag\":{\"IsRec\":true},\"Rack\":{\"IsRec\":true},\"Danger\":{\"IsRec\":true},\"Crash\":{\"IsRec\":true},\"Call\":{\"IsRec\":true},\"Belt\":{\"IsRec\":true},\"Convertible\":{\"IsRec\":true},\"Manned\":{\"IsRec\":true}}}}";
	_result.resize(4 * 1024 * 1024);
}

bool RecognChannel::Inited()
{
	return _inited;
}

void RecognChannel::PushGuids(const vector<string>& guids)
{
	if (!guids.empty()&&_guids.size()< MaxCacheCount)
	{
		lock_guard<mutex> lck(_mutex);
		for (vector<string>::const_iterator it = guids.begin(); it != guids.end(); ++it)
		{
			_guids.push(*it);
		}
	}
}

void RecognChannel::HandleRecognize(const string& json)
{
	long long timeStamp = DateTime::TimeStamp();
	JsonDeserialization jd(json);
	vector<string> images = jd.GetArray<string>("imgdata_result");
	unsigned int imageIndex = 0;
	while (imageIndex < images.size())
	{
		int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Type"));
		if (vehicleType != 0)
		{
			vector<int> vehicleRect = jd.GetArray<int>(StringEx::Combine("l1_result:", imageIndex, ":Detect:Body:Rect"));
			if (vehicleRect.size() >= 4)
			{
				DetectItem item(Saitama::Rectangle(Point(vehicleRect[0], vehicleRect[1]), vehicleRect[2], vehicleRect[3]));
				lock_guard<mutex> lck(_laneMutex);
				for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
				{
					if (_lanes[laneIndex]->Contains(item))
					{
						VideoVehicle vehicle;
						vehicle.CarType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Type:TopList:0:Code"));
						vehicle.CarColor = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Color:TopList:0:Code"));
						vehicle.CarBrand = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Brand:TopList:0:Name"));
						vehicle.PlateType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Plate:Type"));
						vehicle.PlateNumber = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Plate:Licence"));
						vehicle.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Feature:Feature"));
						vehicle.Image = images[imageIndex];

						string sendJson;
						JsonSerialization::Serialize(&sendJson, "dataId", _lanes[laneIndex]->DataId());
						JsonSerialization::Serialize(&sendJson, "timeStamp", timeStamp);
						JsonSerialization::Serialize(&sendJson, "videoStructType", vehicle.VideoStructType);
						JsonSerialization::Serialize(&sendJson, "feature", vehicle.Feature);
						JsonSerialization::Serialize(&sendJson, "image", vehicle.Image);
						JsonSerialization::Serialize(&sendJson, "carType", vehicle.CarType);
						JsonSerialization::Serialize(&sendJson, "carColor", vehicle.CarColor);
						JsonSerialization::Serialize(&sendJson, "carBrand", vehicle.CarBrand);
						JsonSerialization::Serialize(&sendJson, "plateType", vehicle.PlateType);
						JsonSerialization::Serialize(&sendJson, "plateNumber", vehicle.PlateNumber);
						LogPool::Debug(LogEvent::Detect, "lane:", _lanes[laneIndex]->DataId(), "vehicle:", vehicle.CarType);
						_mqtt->Send(VideoStructTopic, sendJson, true);
						break;
					}
				}

				imageIndex += 1;
				continue;
			}
		}

		int bikeType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Type"));
		if (bikeType != 0)
		{
			vector<int> bikeRect = jd.GetArray<int>(StringEx::Combine("l1_result:", imageIndex, ":Detect:Body:Rect"));
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
						bike.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Recognize:Feature:Feature"));
						bike.Image = images[imageIndex];

						string sendJson;
						JsonSerialization::Serialize(&sendJson, "dataId", _lanes[laneIndex]->DataId());
						JsonSerialization::Serialize(&sendJson, "timeStamp", timeStamp);
						JsonSerialization::Serialize(&sendJson, "videoStructType", bike.VideoStructType);
						JsonSerialization::Serialize(&sendJson, "feature", bike.Feature);
						JsonSerialization::Serialize(&sendJson, "image", bike.Image);
						JsonSerialization::Serialize(&sendJson, "bikeType", bike.BikeType);
						LogPool::Debug(LogEvent::Detect, "lane:", _lanes[laneIndex]->DataId(), "bike:", bike.BikeType);
						_mqtt->Send(VideoStructTopic, sendJson, true);
					}
				}

				imageIndex += 1;
				continue;
			}
		}

		int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Type"));
		if (pedestrainType != 0)
		{
			vector<int> pedestrainRect = jd.GetArray<int>(StringEx::Combine("l1_result:", imageIndex, ":Detect:Body:Rect"));
			if (pedestrainRect.size() >= 4)
			{
				DetectItem item(Saitama::Rectangle(Point(pedestrainRect[0], pedestrainRect[1]), pedestrainRect[2], pedestrainRect[3]));
				lock_guard<mutex> lck(_laneMutex);
				for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
				{
					if (_lanes[laneIndex]->Contains(item))
					{
						VideoPedestrain pedestrain;
						pedestrain.Sex = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Sex:TopList:0:Code"));
						pedestrain.Age = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Age:TopList:0:Code"));
						pedestrain.UpperColor = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
						pedestrain.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Feature:Feature"));
						pedestrain.Image = images[imageIndex];
						string sendJson;
						JsonSerialization::Serialize(&sendJson, "dataId", _lanes[laneIndex]->DataId());
						JsonSerialization::Serialize(&sendJson, "timeStamp", timeStamp);
						JsonSerialization::Serialize(&sendJson, "feature", pedestrain.Feature);
						JsonSerialization::Serialize(&sendJson, "image", pedestrain.Image);
						JsonSerialization::Serialize(&sendJson, "videoStructType", pedestrain.VideoStructType);
						JsonSerialization::Serialize(&sendJson, "sex", pedestrain.Sex);
						JsonSerialization::Serialize(&sendJson, "age", pedestrain.Age);
						JsonSerialization::Serialize(&sendJson, "upperColor", pedestrain.UpperColor);
						LogPool::Debug(LogEvent::Detect, "lane:", _lanes[laneIndex]->DataId(), "pedestrain");
						_mqtt->Send(VideoStructTopic, sendJson, true);
					}
				}
				imageIndex += 1;
				continue;
			}
		}
	}
}

void RecognChannel::StartCore()
{
	if (SeemmoSDK::seemmo_thread_init == NULL)
	{
		return;
	}
	else
	{
		int result = SeemmoSDK::seemmo_thread_init(2, _channelIndex%2, ItemCount);
		if (result == 0)
		{
			LogPool::Information(LogEvent::Detect, "init recogn thread sucess");
		}
		else
		{
			LogPool::Warning(LogEvent::Detect, "init thread failed", result);
			return;
		}
	}
	_inited = true;
	while (!_cancelled)
	{
		if (_guids.empty())
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
		else
		{
			lock_guard<mutex> lck(_mutex);
			string guid = _guids.front();
			_guids.pop();
			_guidValues[0] = guid.c_str();
			int32_t size = static_cast<int32_t>(_result.size());
			int result=SeemmoSDK::seemmo_video_pvc_recog(static_cast<int32_t>(_guidValues.size())
				, _guidValues.data()
				, _recognParam.c_str()
				, _result.data()
				, &size
				, _recognDataValue.data()
				, 0);
			if (result == 0)
			{
				HandleRecognize(_result.data());
			}
		}
	}

	if (SeemmoSDK::seemmo_thread_uninit != NULL)
	{
		SeemmoSDK::seemmo_thread_uninit();
	}
}