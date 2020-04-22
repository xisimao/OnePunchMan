#include "RecognChannel.h"

using namespace std;
using namespace Saitama;
using namespace TerribleTornado;

const int RecognChannel::ItemCount = 4;
const int RecognChannel::MaxCacheCount = 20;
const int RecognChannel::SleepTime = 50;

RecognChannel::RecognChannel(int recognIndex,int width, int height, const vector<ChannelDetector*>& detectors)
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

void RecognChannel::PushGuids(int channelIndex,const vector<string>& guids)
{
	if (!guids.empty())
	{
		if (_guidsCache.size() < MaxCacheCount)
		{
			lock_guard<mutex> lck(_mutex);
			for (vector<string>::const_iterator it = guids.begin(); it != guids.end(); ++it)
			{
				GuidItem item;
				item.ChannelIndex = channelIndex;
				item.Guid = *it;
				_guidsCache.push(item);
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
		if (_guidsCache.empty())
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
		else
		{
			lock_guard<mutex> lck(_mutex);
			GuidItem guid = _guidsCache.front();
			_guidsCache.pop();
			_guids[0] = guid.Guid.c_str();
			int32_t size = static_cast<int32_t>(_result.size());
			int result=SeemmoSDK::seemmo_video_pvc_recog(1
				, _guids.data()
				, _param.c_str()
				, _result.data()
				, &size
				, _bgrs.data()
				, 0);
			if (result == 0)
			{
				LogPool::Information(_result.data());
				_detectors[guid.ChannelIndex-1]->HandleRecognize(_result.data());
			}
		}
	}

	if (SeemmoSDK::seemmo_thread_uninit != NULL)
	{
		SeemmoSDK::seemmo_thread_uninit();
	}
}
