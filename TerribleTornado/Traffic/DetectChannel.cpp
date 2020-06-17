#include "DetectChannel.h"

using namespace std;
using namespace OnePunchMan;

const int DetectChannel::SleepTime=40;

DetectChannel::DetectChannel(int detectIndex, int width, int height)
	:ThreadObject("detect"), _inited(false), _detectIndex(detectIndex), _width(width), _height(height), _recogn(NULL)
{
	_indexes.resize(1);
	_timeStamps.resize(1);
	_ives.resize(1);
	_heights.push_back(_height);
	_widths.push_back(_width);
	_params.resize(1);
	_result.resize(4 * 1024 * 1024);
}

bool DetectChannel::Inited()
{
	return _inited;
}

void DetectChannel::SetRecogn(RecognChannel* recogn)
{
	_recogn = recogn;
}

void DetectChannel::AddChannel(int channelIndex, DecodeChannel* decode, TrafficDetector* detector)
{
	LogPool::Information(LogEvent::System, "detect", _detectIndex, "add channel", channelIndex);
	ChannelItem item;
	item.ChannelIndex = channelIndex;
	item.Param = TrafficDetector::DefaultDetectParam;
	item.Decode = decode;
	item.Detector = detector;
	_channelItems.push_back(item);
}

void DetectChannel::GetDetecItems(map<string, DetectItem>* items, const JsonDeserialization& jd, const string& key)
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
			DetectItem item;
			item.Region = Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]);
			item.Type = static_cast<DetectType>(type);
			items->insert(pair<string, DetectItem>(id, item));
		}
		itemIndex += 1;
	}
}

void DetectChannel::GetRecognItems(vector<RecognItem>* items, const JsonDeserialization& jd, const string& key,int channelIndex)
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
			item.ChannelIndex = channelIndex;
			item.Guid = id;
			item.Type = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Type"));
			item.Width = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Width"));
			item.Height = jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Detect:Body:Height"));
			item.Region = Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]);
			items->push_back(item);
		}
		itemIndex += 1;
	}
}

void DetectChannel::StartCore()
{
	if (SeemmoSDK::seemmo_thread_init == NULL || SeemmoSDK::seemmo_video_pvc == NULL || SeemmoSDK::seemmo_video_pvc_recog == NULL)
	{
		return;
	}
	else
	{
		int result = SeemmoSDK::seemmo_thread_init(1, _detectIndex % 2, 1);
		if (result == 0)
		{
			LogPool::Information(LogEvent::Detect, "init detect thread success");
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
		bool detected = false;
		for (unsigned int i = 0; i < _channelItems.size(); ++i)
		{
			ChannelItem& channelItem = _channelItems[i];
			long long detectTimeStamp = DateTime::UtcNowTimeStamp();
			FrameItem frameItem = channelItem.Decode->GetTempIve();
			if (frameItem.IveBuffer != NULL)
			{
				detected = true;
				_indexes[0] = channelItem.ChannelIndex;
				_timeStamps[0] = frameItem.FrameIndex;
				_ives[0] = frameItem.IveBuffer;
				_params[0] = channelItem.Param.c_str();
				long long detectTimeStamp1 = DateTime::UtcNowTimeStamp();
				int32_t size = static_cast<int32_t>(_result.size());
				int result = SeemmoSDK::seemmo_video_pvc(1,
					_indexes.data(),
					_timeStamps.data(),
					const_cast<const std::uint8_t**>(_ives.data()),
					_heights.data(),
					_widths.data(),
					_params.data(),
					_result.data(),
					&size,
					0);
				long long detectTimeStamp2 = DateTime::UtcNowTimeStamp();
				if (result == 0)
				{
					JsonDeserialization detectJd(_result.data());
					if (_recogn != NULL)
					{
						vector<RecognItem> recognItems;
						GetRecognItems(&recognItems, detectJd, "Vehicles", channelItem.ChannelIndex);
						GetRecognItems(&recognItems, detectJd, "Bikes", channelItem.ChannelIndex);
						GetRecognItems(&recognItems, detectJd, "Pedestrains", channelItem.ChannelIndex);
						if (!recognItems.empty())
						{
							_recogn->PushItems(recognItems);
						}
					}
					map<string, DetectItem> detectItems;
					GetDetecItems(&detectItems, detectJd, "Vehicles");
					GetDetecItems(&detectItems, detectJd, "Bikes");
					GetDetecItems(&detectItems, detectJd, "Pedestrains");
					channelItem.Detector->HandleDetect(&detectItems, detectTimeStamp, &channelItem.Param, _ives[0], static_cast<int>(_timeStamps[0]), frameItem.FrameSpan);
				}
				long long detectTimeStamp3 = DateTime::UtcNowTimeStamp();
				LogPool::Debug("detect", _indexes[0], _timeStamps[0], frameItem.FrameSpan, result, detectTimeStamp1 - detectTimeStamp, detectTimeStamp2 - detectTimeStamp1, detectTimeStamp3 - detectTimeStamp2);
			}

			if (frameItem.Finished)
			{
				channelItem.Detector->FinishDetect();
			}
		}
		if (!detected)
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
	}

	if (SeemmoSDK::seemmo_thread_uninit != NULL)
	{
		SeemmoSDK::seemmo_thread_uninit();
	}
}

