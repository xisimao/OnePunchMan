#include "DetectChannel.h"

using namespace std;
using namespace OnePunchMan;

const int DetectChannel::SleepTime=40;

DetectChannel::DetectChannel(int detectIndex, int channelCount, int width, int height, RecognChannel* recogn, const vector<TrafficDetector*>& detectors)
	:ThreadObject("detect"), _inited(false), _detectIndex(detectIndex),_channelCount(channelCount), _width(width), _height(height)
	, _recogn(recogn),_detectors(detectors), _iveHandler(-1)
{
	for (int i = 0; i < channelCount; ++i)
	{
		FrameItem item;
		item.ChannelIndex = detectIndex * channelCount + i+1;
		item.YuvSize = static_cast<int>(_width * _height * 1.5);
		item.TempHasValue = false;
		item.YuvTempBuffer = new uint8_t[item.YuvSize];
		item.IveSize = _width * _height * 3;
#ifndef _WIN32
		if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&item.Yuv_phy_addr),
			reinterpret_cast<HI_VOID**>(&item.YuvBuffer),
			"yuv_buffer",
			NULL,
			item.YuvSize) != HI_SUCCESS) {
			LogPool::Error(LogEvent::Detect,"HI_MPI_SYS_MmzAlloc_Cached yuv");
			exit(2);
		}
		if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&item.Ive_phy_addr),
			reinterpret_cast<HI_VOID**>(&item.IveBuffer),
			"ive_buffer",
			NULL,
			item.IveSize) != HI_SUCCESS) {
			LogPool::Error(LogEvent::Detect, "HI_MPI_SYS_MmzAlloc_Cached ive");
			exit(2);
		}
#endif // !_WIN32
		item.Param = "{\"Detect\":{\"DetectRegion\":[],\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}";
		_frameItems.push_back(item);
	}
	_indexes.resize(1);
	_timeStamps.resize(1);
	_ives.resize(1);
	_heights.push_back(_height);
	_widths.push_back(_width);
	_params.resize(1);
	_result.resize(4 * 1024 * 1024);
}

DetectChannel::~DetectChannel()
{
	for (int i = 0; i < _channelCount; ++i)
	{
		delete[] _frameItems[i].YuvTempBuffer;
#ifndef _WIN32
		HI_MPI_SYS_MmzFree(_frameItems[i].Yuv_phy_addr, reinterpret_cast<HI_VOID*>(_frameItems[i].YuvBuffer));
		HI_MPI_SYS_MmzFree(_frameItems[i].Ive_phy_addr, reinterpret_cast<HI_VOID*>(_frameItems[i].IveBuffer));
#endif // !_WIN32
	}
}

bool DetectChannel::Inited()
{
	return _inited;
}

int DetectChannel::GetFrameItemIndex(int channelIndex)
{
	unsigned int itemIndex = (channelIndex - 1) % _channelCount;
	if (itemIndex >= 0 && itemIndex < _frameItems.size())
	{
		return static_cast<int>(itemIndex);
	}
	else
	{
		return -1;
	}
}

void DetectChannel::WriteBmp(int channelIndex)
{
	int itemIndex= GetFrameItemIndex(channelIndex);
	if (itemIndex!=-1)
	{
		_frameItems[itemIndex].WriteBmp = true;
	}
}

bool DetectChannel::IsBusy(int channelIndex)
{ 
	int itemIndex = GetFrameItemIndex(channelIndex);
	if (itemIndex == -1)
	{
		return true;
	}
	else
	{
		return _frameItems[itemIndex].TempHasValue || !_inited || (_recogn != NULL && !_recogn->Inited());
	}
}

void DetectChannel::HandleYUV(int channelIndex, const unsigned char* yuv, int width, int height, int frameIndex, int frameSpan)
{
	int itemIndex = GetFrameItemIndex(channelIndex);
	if (itemIndex != -1)
	{
		memcpy(_frameItems[itemIndex].YuvTempBuffer, yuv, _frameItems[itemIndex].YuvSize);
		_frameItems[itemIndex].FrameTempIndex = frameIndex;
		_frameItems[itemIndex].FrameSpan = frameSpan;
		_frameItems[itemIndex].TempHasValue = true;
	}
}

bool DetectChannel::YuvToIve(FrameItem* frameItem)
{
#ifndef _WIN32
	IVE_IMAGE_S yuv_image_list;
	IVE_IMAGE_S bgr_image_list;

	IVE_HANDLE ive_handle;
	IVE_CSC_CTRL_S ive_csc_ctrl = { IVE_CSC_MODE_PIC_BT709_YUV2RGB };
	int hi_s32_ret = HI_SUCCESS;

	yuv_image_list.enType = IVE_IMAGE_TYPE_YUV420SP;
	yuv_image_list.u32Height = _height;
	yuv_image_list.u32Width = _width;
	yuv_image_list.au64PhyAddr[0] = frameItem->Yuv_phy_addr;
	yuv_image_list.au64PhyAddr[1] = yuv_image_list.au64PhyAddr[0] + yuv_image_list.u32Width * yuv_image_list.u32Height;
	yuv_image_list.au32Stride[0] = yuv_image_list.u32Width;
	yuv_image_list.au32Stride[1] = yuv_image_list.u32Width;

	yuv_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(frameItem->YuvBuffer);
	yuv_image_list.au64VirAddr[1] = yuv_image_list.au64VirAddr[0] + yuv_image_list.u32Width * yuv_image_list.u32Height;

	bgr_image_list.enType = IVE_IMAGE_TYPE_U8C3_PLANAR;
	bgr_image_list.u32Height = _height;
	bgr_image_list.u32Width = _width;
	bgr_image_list.au64PhyAddr[0] = frameItem->Ive_phy_addr;
	bgr_image_list.au64PhyAddr[1] = bgr_image_list.au64PhyAddr[0] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64PhyAddr[2] = bgr_image_list.au64PhyAddr[1] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(frameItem->IveBuffer);
	bgr_image_list.au64VirAddr[1] = bgr_image_list.au64VirAddr[0] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64VirAddr[2] = bgr_image_list.au64VirAddr[1] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au32Stride[0] = bgr_image_list.u32Width;
	bgr_image_list.au32Stride[1] = bgr_image_list.u32Width;
	bgr_image_list.au32Stride[2] = bgr_image_list.u32Width;

	hi_s32_ret = HI_MPI_IVE_CSC(&ive_handle, &yuv_image_list, &bgr_image_list, &ive_csc_ctrl, HI_TRUE);
	if (HI_SUCCESS != hi_s32_ret) {
		LogPool::Error(LogEvent::Detect,"HI_MPI_IVE_CSC", hi_s32_ret);
		return false;
	}
	HI_BOOL ive_finish = HI_FALSE;
	hi_s32_ret = HI_SUCCESS;
	do {
		hi_s32_ret = HI_MPI_IVE_Query(ive_handle, &ive_finish, HI_TRUE);
	} while (HI_ERR_IVE_QUERY_TIMEOUT == hi_s32_ret);

	if (HI_SUCCESS != hi_s32_ret) {
		LogPool::Error(LogEvent::Detect, "HI_MPI_IVE_Query", hi_s32_ret);
		return false;
	}

#endif // !_WIN32
	return true;
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
		for (unsigned int i = 0; i < _frameItems.size(); ++i)
		{
			FrameItem& frameItem = _frameItems[i];
			if (frameItem.TempHasValue)
			{
				detected = true;
				long long detectTimeStamp = DateTime::UtcNowTimeStamp();
				memcpy(frameItem.YuvBuffer, frameItem.YuvTempBuffer, frameItem.YuvSize);
				frameItem.FrameIndex = frameItem.FrameTempIndex;
				frameItem.TempHasValue = false;
				if (YuvToIve(&frameItem))
				{
					if (frameItem.WriteBmp)
					{
						_iveHandler.HandleFrame(frameItem.IveBuffer, _width, _height, frameItem.ChannelIndex);
						frameItem.WriteBmp = false;
					}
					_indexes[0]= frameItem.ChannelIndex;
					_timeStamps[0] = frameItem.FrameIndex;
					_ives[0] = frameItem.IveBuffer;
					_params[0] = frameItem.Param.c_str();
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
							GetRecognItems(&recognItems, detectJd, "Vehicles",frameItem.ChannelIndex);
							GetRecognItems(&recognItems, detectJd, "Bikes", frameItem.ChannelIndex);
							GetRecognItems(&recognItems, detectJd, "Pedestrains", frameItem.ChannelIndex);
							if (!recognItems.empty())
							{
								_recogn->PushItems(recognItems);
							}
						}
						map<string, DetectItem> detectItems;
						GetDetecItems(&detectItems, detectJd, "Vehicles");
						GetDetecItems(&detectItems, detectJd, "Bikes");
						GetDetecItems(&detectItems, detectJd, "Pedestrains");
						_detectors[frameItem.ChannelIndex-1]->HandleDetect(&detectItems, detectTimeStamp, &frameItem.Param, _ives[0], static_cast<int>(_timeStamps[0]), frameItem.FrameSpan);
					}
					long long detectTimeStamp3 = DateTime::UtcNowTimeStamp();
					LogPool::Debug("detect", _indexes[0], _timeStamps[0], result, detectTimeStamp1 - detectTimeStamp, detectTimeStamp2 - detectTimeStamp1, detectTimeStamp3 - detectTimeStamp2);
				}
				else
				{
					LogPool::Warning(LogEvent::Detect, "yuv to bgr failed");
				}
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

