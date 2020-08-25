#include "RecognChannel.h"

using namespace std;
using namespace OnePunchMan;

const int RecognChannel::MaxCacheCount = 100;
const int RecognChannel::SleepTime = 500;
const string RecognChannel::RecognTopic("Recogn");

RecognChannel::RecognChannel(int recognIndex, int width, int height, vector<TrafficDetector*>* detectors)
	:ThreadObject("recogn"), _inited(false), _recognIndex(recognIndex), _detectors(detectors)
{
	_bgrs.push_back(new uint8_t[width * height * 3]);
	_guids.resize(1);
	_param = "{\"Detect\":{\"IsDet\":true,\"Mode\":0,\"Threshold\":20},\"Recognize\":{\"Person\":{\"IsRec\":true},\"Vehicle\":{\"Brand\":{\"IsRec\":true},\"Plate\":{\"IsRec\":true},\"Color\":{\"IsRec\":true}}}}";
	_result.resize(4 * 1024 * 1024);
}

RecognChannel::~RecognChannel()
{
	delete[] _bgrs[0];
}

bool RecognChannel::Inited()
{
	return _inited;
}

int RecognChannel::Size()
{
	return static_cast<int>(_items.size());
}

void RecognChannel::PushItems(const vector<RecognItem>& items)
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
			LogPool::Warning(LogEvent::Recogn, "识别项队列已满，配置的最大数量:",MaxCacheCount);
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
		int result = SeemmoSDK::seemmo_thread_init(2, _recognIndex %2, 1);
		if (result == 0)
		{
			LogPool::Information(LogEvent::Detect, "初始化seemmo识别线程成功");
		}
		else
		{
			LogPool::Warning(LogEvent::Detect, "初始化seemmo识别线程失败", result);
			return;
		}
	}
	_inited = true;
	while (!_cancelled)
	{
		if (_items.empty())
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
		else
		{
			long long recognTimeStamp1 = DateTime::UtcNowTimeStamp();
			unique_lock<mutex> lck(_queueMutex);
			RecognItem item = _items.front();
			_items.pop();
			lck.unlock();
			_guids[0] = item.Guid.c_str();
			int32_t size = static_cast<int32_t>(_result.size());
			int result = SeemmoSDK::seemmo_video_pvc_recog(1
				, _guids.data()
				, _param.c_str()
				, _result.data()
				, &size
				, _bgrs.data()
				, 0);
			long long recognTimeStamp2 = DateTime::UtcNowTimeStamp();
			if (result == 0)
			{
				if (item.Type == static_cast<int>(DetectType::Car)
					|| item.Type == static_cast<int>(DetectType::Tricycle)
					|| item.Type == static_cast<int>(DetectType::Bus)
					|| item.Type == static_cast<int>(DetectType::Van)
					|| item.Type == static_cast<int>(DetectType::Truck))
				{
					JsonDeserialization jd(_result.data());
					int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Type"));
					if (vehicleType != 0)
					{
						VideoStruct_Vehicle vehicle;
						vehicle.CarType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Type:TopList:0:Code"));
						vehicle.CarColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Color:TopList:0:Code"));
						vehicle.CarBrand = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Brand:TopList:0:Name"));
						vehicle.PlateType = jd.Get<int>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Type"));
						vehicle.PlateNumber = jd.Get<string>(StringEx::Combine("ImageResults:0:Vehicles:0:Recognize:Plate:Licence"));
						_detectors->at(item.ChannelIndex - 1)->HandleRecognVehicle(item, _bgrs[0], vehicle);
					}
				}
				else if (item.Type == static_cast<int>(DetectType::Bike)
					|| item.Type == static_cast<int>(DetectType::Motobike))
				{
					JsonDeserialization jd(_result.data());
					int bikeType = jd.Get<int>(StringEx::Combine("ImageResults:0:Bikes:0:Type"));
					if (bikeType != 0)
					{
						VideoStruct_Bike bike;
						bike.BikeType = bikeType;
						_detectors->at(item.ChannelIndex - 1)->HandleRecognBike(item, _bgrs[0], bike);
					}
				}
				else if (item.Type == static_cast<int>(DetectType::Pedestrain))
				{
					JsonDeserialization jd(_result.data());
					int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Type"));
					if (pedestrainType != 0)
					{
						VideoStruct_Pedestrain pedestrain;
						pedestrain.Sex = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Sex:TopList:0:Code"));
						pedestrain.Age = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:Age:TopList:0:Code"));
						pedestrain.UpperColor = jd.Get<int>(StringEx::Combine("ImageResults:0:Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
						_detectors->at(item.ChannelIndex - 1)->HandleRecognPedestrain(item, _bgrs[0], pedestrain);
					}
				}
			}
			long long recognTimeStamp3 = DateTime::UtcNowTimeStamp();
			LogPool::Debug("recogn", item.ChannelIndex, result, recognTimeStamp2 - recognTimeStamp1, recognTimeStamp3 - recognTimeStamp2);
		}
	}

	if (SeemmoSDK::seemmo_thread_uninit != NULL)
	{
		SeemmoSDK::seemmo_thread_uninit();
	}
}
