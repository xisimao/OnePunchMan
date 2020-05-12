#include "RecognChannel.h"

using namespace std;
using namespace OnePunchMan;

const int RecognChannel::ItemCount = 4;
const int RecognChannel::MaxCacheCount = 100;
const int RecognChannel::SleepTime = 100;

RecognChannel::RecognChannel(int recognIndex,int width, int height, const vector<TrafficDetector*>& detectors)
	:ThreadObject("recogn"), _inited(false), _recognIndex(recognIndex),_detectors(detectors)
{
	_bgrs.push_back(new uint8_t[width * height * 3]);
	_guids.resize(1);
	_param = "{\"Detect\":{\"DetectRegion\":[],\"IsDet\":true,\"MaxCarWidth\":0,\"MinCarWidth\":0,\"Mode\":0,\"Threshold\":20,\"Version\":1001},\"Recognize\":{\"Person\":{\"IsRec\":true},\"Feature\":{\"IsRec\":true},\"Vehicle\":{\"Brand\":{\"IsRec\":true},\"Plate\":{\"IsRec\":true},\"Color\":{\"IsRec\":true},\"Marker\":{\"IsRec\":true},\"Sunroof\":{\"IsRec\":true},\"SpareTire\":{\"IsRec\":true},\"Slag\":{\"IsRec\":true},\"Rack\":{\"IsRec\":true},\"Danger\":{\"IsRec\":true},\"Crash\":{\"IsRec\":true},\"Call\":{\"IsRec\":true},\"Belt\":{\"IsRec\":true},\"Convertible\":{\"IsRec\":true},\"Manned\":{\"IsRec\":true}}}}";
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

void RecognChannel::PushItems(const vector<RecognItem> items)
{
	if (!items.empty())
	{
		if (_items.size() < MaxCacheCount)
		{
			lock_guard<mutex> lck(_queueMutex);
			for (vector<RecognItem>::const_iterator it = items.begin(); it != items.end(); ++it)
			{
				_items.push(*it);
			}
		}
		else
		{
			LogPool::Warning(LogEvent::Recogn, "too many guids",MaxCacheCount);
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
		int result = SeemmoSDK::seemmo_thread_init(2, _recognIndex %2, ItemCount);
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
			int result=SeemmoSDK::seemmo_video_pvc_recog(1
				, _guids.data()
				, _param.c_str()
				, _result.data()
				, &size
				, _bgrs.data()
				, 0);
			long long recognTimeStamp2= DateTime::UtcNowTimeStamp();

			if (result == 0)
			{
				_detectors[item.ChannelIndex-1]->HandleRecognize(item,_bgrs[0],_result.data());
			}
			long long recognTimeStamp3 = DateTime::UtcNowTimeStamp();

			LogPool::Debug("recogn", item.ChannelIndex, recognTimeStamp3 - recognTimeStamp1, recognTimeStamp3 - recognTimeStamp2, recognTimeStamp2 - recognTimeStamp1);
		}
	}

	if (SeemmoSDK::seemmo_thread_uninit != NULL)
	{
		SeemmoSDK::seemmo_thread_uninit();
	}
}
