#include "DetectChannel.h"

using namespace std;
using namespace OnePunchMan;

const int DetectChannel::SleepTime=40;

DetectChannel::DetectChannel(int detectIndex, int width, int height)
	:ThreadObject("detect"), _inited(false), _detectIndex(detectIndex), _width(width), _height(height), _recogn(NULL), _image(width,height,false)
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

void DetectChannel::AddChannel(int channelIndex, DecodeChannel* decode, FlowDetector* flowDetector, EventDetector* eventDetector)
{
	LogPool::Debug(LogEvent::Detect, "No.", channelIndex, "channel add to the",_detectIndex+1,"detect thread");
	ChannelItem item;
	item.ChannelIndex = channelIndex;
	item.Param = "[]";
	item.Decode = decode;
	item.Flow = flowDetector;
	item.Event = eventDetector;
	_channelItems.insert(pair<int,ChannelItem>(channelIndex,item));
}

void DetectChannel::UpdateChannel(const TrafficChannel& channel)
{
	lock_guard<mutex> lck(_channelMutex);
	map<int, ChannelItem>::iterator it = _channelItems.find(channel.ChannelIndex);
	if (it != _channelItems.end())
	{
		if (channel.GlobalDetect)
		{
			it->second.Param = "[]";
		}
		else
		{
			string regionsParam;
			regionsParam.append("[");
			for (vector<FlowLane>::const_iterator it = channel.FlowLanes.begin(); it != channel.FlowLanes.end(); ++it)
			{
				if (it->QueueRegion.size() > 2)
				{
					regionsParam.append(it->QueueRegion);
					regionsParam.append(",");
				}
			}
			if (regionsParam.size() == 1)
			{
				regionsParam.append("]");
			}
			else
			{
				regionsParam[regionsParam.size() - 1] = ']';
			}
			it->second.Param = StringEx::Combine("{\"Detect\":{\"DetectRegion\":", regionsParam, ",\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}");
		}	

		it->second.WriteBmp = true;
		it->second.GlobalDetect = channel.GlobalDetect;
		remove(StringEx::Combine(TrafficDirectory::FileDir, "channel_", channel.ChannelIndex, ".bmp").c_str());
	}
}

void DetectChannel::WriteBmp(int channelIndex)
{
	lock_guard<mutex> lck(_channelMutex);
	map<int, ChannelItem>::iterator it = _channelItems.find(channelIndex);
	if (it != _channelItems.end())
	{
		it->second.WriteBmp = true;
	}
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
			item.Id = id;
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
			LogPool::Information(LogEvent::Detect, "初始化Seemmo检测线程");
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
		unique_lock<mutex> lck(_channelMutex);
		bool detected = false;
		for (map<int,ChannelItem>::iterator it=_channelItems.begin();it!=_channelItems.end();++it)
		{
			long long detectTimeStamp = DateTime::NowTimeStamp();
			FrameItem frameItem = it->second.Decode->GetIve();

			if (frameItem.Finished)
			{
				it->second.Flow->FinishDetect(frameItem.TaskId);
			}
			else if(frameItem.IveBuffer != NULL)
			{
				if (it->second.WriteBmp)
				{
					_image.IveToJpgFile(frameItem.IveBuffer,_width, _height, StringEx::Combine(TrafficDirectory::FileDir, "channel_", it->second.ChannelIndex, ".jpg"));
					it->second.WriteBmp = false;
				}
				detected = true;
				_indexes[0] = it->second.ChannelIndex;
				_timeStamps[0] = frameItem.FrameIndex;
				_ives[0] = frameItem.IveBuffer;
				_params[0] = it->second.Param.c_str();
				long long detectTimeStamp1 = DateTime::NowTimeStamp();
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
				long long detectTimeStamp2 = DateTime::NowTimeStamp();
				if (result == 0)
				{
					JsonDeserialization detectJd(_result.data());
					if (_recogn != NULL)
					{
						vector<RecognItem> recognItems;
						GetRecognItems(&recognItems, detectJd, "Vehicles", it->second.ChannelIndex, frameItem.FrameIndex, frameItem.FrameSpan,frameItem.TaskId);
						GetRecognItems(&recognItems, detectJd, "Bikes", it->second.ChannelIndex, frameItem.FrameIndex, frameItem.FrameSpan, frameItem.TaskId);
						GetRecognItems(&recognItems, detectJd, "Pedestrains", it->second.ChannelIndex, frameItem.FrameIndex, frameItem.FrameSpan, frameItem.TaskId);
						if (!recognItems.empty())
						{
							_recogn->PushItems(recognItems);
						}
					}
					map<string, DetectItem> detectItems;
					GetDetecItems(&detectItems, detectJd, "Vehicles");
					GetDetecItems(&detectItems, detectJd, "Bikes");
					GetDetecItems(&detectItems, detectJd, "Pedestrains");

					if (it->second.GlobalDetect && !detectItems.empty())
					{
						string json;
						JsonSerialization::SerializeValue(&json,"channelIndex", it->second.ChannelIndex);
						JsonSerialization::SerializeValue(&json,"taskId", static_cast<int>(frameItem.TaskId));
						JsonSerialization::SerializeValue(&json,"frameIndex", frameItem.FrameIndex);
						JsonSerialization::SerializeValue(&json,"frameSpan", static_cast<int>(frameItem.FrameSpan));
						string itemsJson;
						for (map<string, DetectItem>::iterator it = detectItems.begin(); it != detectItems.end(); ++it)
						{
							JsonSerialization::AddClassItem(&itemsJson, it->second.ToJson());
						}
						JsonSerialization::SerializeClass(&json, "items", itemsJson);
						LogPool::Information(LogEvent::DetectData, json);
						_image.IveToJpgFile(frameItem.IveBuffer, _width, _height, StringEx::Combine(TrafficDirectory::TempDir, it->second.ChannelIndex, "_", frameItem.FrameIndex, ".jpg"));
					}

					it->second.Flow->HandleDetect(&detectItems, detectTimeStamp, frameItem.TaskId, frameItem.IveBuffer, frameItem.FrameIndex, frameItem.FrameSpan);
					it->second.Event->HandleDetect(&detectItems, detectTimeStamp, frameItem.TaskId, frameItem.IveBuffer, frameItem.FrameIndex, frameItem.FrameSpan);
				}
				long long detectTimeStamp3 = DateTime::NowTimeStamp();
				if (index % 100 == 0)
				{
					LogPool::Debug(LogEvent::Detect, "detect->channel index:", it->second.ChannelIndex, "task id:",static_cast<int>(frameItem.TaskId), "frame index:", frameItem.FrameIndex, "result:", result, " get frame:", detectTimeStamp1 - detectTimeStamp, " sdk:", detectTimeStamp2 - detectTimeStamp1, " traffic:", detectTimeStamp3 - detectTimeStamp2);
				}				
			}
		}
		lck.unlock();
		if (detected)
		{
			index += 1;
			this_thread::sleep_for(chrono::milliseconds(10));
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

