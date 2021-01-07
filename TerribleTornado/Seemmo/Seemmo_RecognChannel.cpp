#include "Seemmo_RecognChannel.h"

using namespace std;
using namespace OnePunchMan;

const int Seemmo_RecognChannel::MaxCacheCount = 100;
const int Seemmo_RecognChannel::SleepTime = 500;
const string Seemmo_RecognChannel::RecognTopic("Recogn");

Seemmo_RecognChannel::Seemmo_RecognChannel(int recognIndex, int width, int height, vector<FlowDetector*>* detectors)
	:ThreadObject("recogn"), _inited(false), _recognIndex(recognIndex), _detectors(detectors)
{
	_bgrs.push_back(new uint8_t[width * height * 3]);
	_guids.resize(1);
	_param = "{\"Detect\":{\"IsDet\":true,\"Mode\":0,\"Threshold\":20},\"Recognize\":{\"Person\":{\"IsRec\":true},\"Vehicle\":{\"Brand\":{\"IsRec\":true},\"Plate\":{\"IsRec\":true},\"Color\":{\"IsRec\":true}}}}";
	_result.resize(4 * 1024 * 1024);
}

Seemmo_RecognChannel::~Seemmo_RecognChannel()
{
	delete[] _bgrs[0];
}

bool Seemmo_RecognChannel::Inited()
{
	return _inited;
}

int Seemmo_RecognChannel::Size()
{
	return static_cast<int>(_items.size());
}

void Seemmo_RecognChannel::PushItems(const vector<RecognItem>& items)
{
	if (!items.empty())
	{
		if (_items.size() < MaxCacheCount)
		{
			unique_lock<mutex> lck(_queueMutex);
			for (vector<RecognItem>::const_iterator it = items.begin(); it != items.end(); ++it)
			{
				_items.push(*it);
			}
		}
		else
		{
			LogPool::Warning(LogEvent::Recogn, "recogn queue is full,config max count:",MaxCacheCount);
		}
	}
}

void Seemmo_RecognChannel::StartCore()
{
	if (Seemmo_SDK::seemmo_thread_init == NULL)
	{
		return;
	}
	else
	{
		int result = Seemmo_SDK::seemmo_thread_init(2, _recognIndex %2, 1);
		if (result == 0)
		{
			LogPool::Information(LogEvent::Recogn, "初始化Seemmo识别线程");
		}
		else
		{
			LogPool::Warning(LogEvent::Recogn, "init seemmo recogn thread failed", result);
			return;
		}
	}
	_inited = true;
	int index = 0;
	while (!_cancelled)
	{
		if (_items.empty())
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
		else
		{
			long long recognTimeStamp1 = DateTime::NowTimeStamp();
			unique_lock<mutex> lck(_queueMutex);
			RecognItem item = _items.front();
			_items.pop();
			lck.unlock();
			_guids[0] = item.Guid.c_str();
			int32_t size = static_cast<int32_t>(_result.size());
			int result = Seemmo_SDK::seemmo_video_pvc_recog(1
				, _guids.data()
				, _param.c_str()
				, _result.data()
				, &size
				, _bgrs.data()
				, 0);
			long long recognTimeStamp2 = DateTime::NowTimeStamp();
			if (result == 0)
			{
				if (item.Type == DetectType::Car
					|| item.Type == DetectType::Tricycle
					|| item.Type == DetectType::Bus
					|| item.Type == DetectType::Van
					|| item.Type == DetectType::Truck)
				{
					JsonDeserialization jd(_result.data());
					int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Type"));
					if (vehicleType != 0)
					{
						VehicleData vehicle;
						vehicle.CarType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Type:TopList:0:Code"));
						vehicle.CarColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Color:TopList:0:Code"));
						vehicle.CarBrand = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Brand:TopList:0:Name"));
						vehicle.PlateType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Type"));
						vehicle.PlateNumber = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Licence"));
						_detectors->at(item.ChannelIndex - 1)->HandleRecognVehicle(item, _bgrs[0], &vehicle);
					}
				}
				else if (item.Type == DetectType::Bike
					|| item.Type == DetectType::Motobike)
				{
					JsonDeserialization jd(_result.data());
					int bikeType = jd.Get<int>(StringEx::Combine("ImageResults:0:Bikes:0:Type"));
					if (bikeType != 0)
					{
						BikeData bike;
						bike.BikeType = bikeType;
						_detectors->at(item.ChannelIndex - 1)->HandleRecognBike(item, _bgrs[0], &bike);
					}
				}
				else if (item.Type == DetectType::Pedestrain)
				{
					JsonDeserialization jd(_result.data());
					int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Type"));
					if (pedestrainType != 0)
					{
						PedestrainData pedestrain;
						pedestrain.Sex = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Sex:TopList:0:Code"));
						pedestrain.Age = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Age:TopList:0:Code"));
						pedestrain.UpperColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
						_detectors->at(item.ChannelIndex - 1)->HandleRecognPedestrain(item, _bgrs[0], &pedestrain);
					}
				}
				long long recognTimeStamp3 = DateTime::NowTimeStamp();
				if (index % 100 == 0)
				{
					LogPool::Debug(LogEvent::Recogn, "recogn->channel index:", item.ChannelIndex,"result:", result, " sdk:", recognTimeStamp2 - recognTimeStamp1, " traffic:", recognTimeStamp3 - recognTimeStamp2);
				}				
			}
			index += 1;
		}
	}

	if (Seemmo_SDK::seemmo_thread_uninit != NULL)
	{
		Seemmo_SDK::seemmo_thread_uninit();
	}
}
