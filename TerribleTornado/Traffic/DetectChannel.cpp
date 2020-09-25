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
	LogPool::Information(LogEvent::System, "the", channelIndex, "channel add to the",_detectIndex+1,"detect thread");
	ChannelItem item;
	item.ChannelIndex = channelIndex;
	item.Param = detector->GetDetectParam();
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

void DetectChannel::GetRecognItems(vector<RecognItem>* items, const JsonDeserialization& jd, const string& key,int channelIndex,int frameIndex,unsigned char frameSpan,unsigned char taskId)
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
			item.FrameIndex = frameIndex;
			item.FrameSpan = frameSpan;
			item.TaskId = taskId;
			item.Guid = id;
			item.Type = static_cast<DetectType>(jd.Get<int>(StringEx::Combine("FilterResults", ":0:", key, ":", itemIndex, ":Type")));
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
			LogPool::Information(LogEvent::Detect, "init seemmo thread");
		}
		else
		{
			LogPool::Warning(LogEvent::Detect, "init seemmo thread failed", result);
			return;
		}
	}
	_inited = true;
	int index = 0;
	while (!_cancelled)
	{
		bool detected = false;
		for (unsigned int i = 0; i < _channelItems.size(); ++i)
		{
			ChannelItem& channelItem = _channelItems[i];
			long long detectTimeStamp = DateTime::UtcNowTimeStamp();
			FrameItem frameItem = channelItem.Decode->GetTempIve();
			if (frameItem.Finished)
			{
				channelItem.Detector->FinishDetect(frameItem.TaskId);
			}
			else if(frameItem.IveBuffer != NULL)
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
						GetRecognItems(&recognItems, detectJd, "Vehicles", channelItem.ChannelIndex, frameItem.FrameIndex, frameItem.FrameSpan,frameItem.TaskId);
						GetRecognItems(&recognItems, detectJd, "Bikes", channelItem.ChannelIndex, frameItem.FrameIndex, frameItem.FrameSpan, frameItem.TaskId);
						GetRecognItems(&recognItems, detectJd, "Pedestrains", channelItem.ChannelIndex, frameItem.FrameIndex, frameItem.FrameSpan, frameItem.TaskId);
						if (!recognItems.empty())
						{
							_recogn->PushItems(recognItems);
						}
					}
					map<string, DetectItem> detectItems;
					GetDetecItems(&detectItems, detectJd, "Vehicles");
					GetDetecItems(&detectItems, detectJd, "Bikes");
					GetDetecItems(&detectItems, detectJd, "Pedestrains");
					channelItem.Detector->HandleDetect(&detectItems, detectTimeStamp, &channelItem.Param, frameItem.TaskId, frameItem.IveBuffer, frameItem.FrameIndex, frameItem.FrameSpan);
				}
				long long detectTimeStamp3 = DateTime::UtcNowTimeStamp();
				if (index % 100 == 0)
				{
					LogPool::Debug(LogEvent::Detect, "detect->channel index:", channelItem.ChannelIndex, "task id:",static_cast<int>(frameItem.TaskId), "frame index:", frameItem.FrameIndex, "result:", result, " get frame:", detectTimeStamp1 - detectTimeStamp, " sdk:", detectTimeStamp2 - detectTimeStamp1, " traffic:", detectTimeStamp3 - detectTimeStamp2);
				}				
			}
		}
		if (detected)
		{
			index += 1;
		}
		else
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
	}

	if (SeemmoSDK::seemmo_thread_uninit != NULL)
	{
		SeemmoSDK::seemmo_thread_uninit();
	}
}

