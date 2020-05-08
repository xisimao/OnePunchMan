#include "DetectChannel.h"

using namespace std;
using namespace OnePunchMan;

const int DetectChannel::SleepTime=10;

DetectChannel::DetectChannel(int channelIndex, int width, int height, RecognChannel* recogn, TrafficDetector* detector)
	:ThreadObject("detect"), _inited(false), _channelIndex(channelIndex), _width(width), _height(height), _recogn(recogn),_detector(detector)
{
#ifndef _WIN32
	_item.YuvSize = static_cast<int>(_width * _height * 1.5);
	if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&_item.Yuv_phy_addr),
		reinterpret_cast<HI_VOID**>(&_item.YuvBuffer),
		"yuv_buffer",
		NULL,
		_item.YuvSize) != HI_SUCCESS) {
		LogPool::Information("HI_MPI_SYS_MmzAlloc_Cached yuv");
		return;
	}
	_item.YuvTempBuffer = new uint8_t[_item.YuvSize];

	_item.IveSize = _width * _height * 3;
	if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&_item.Ive_phy_addr),
		reinterpret_cast<HI_VOID**>(&_item.IveBuffer),
		"bgr_buffer",
		NULL,
		_item.IveSize) != HI_SUCCESS) {
		LogPool::Information("HI_MPI_SYS_MmzAlloc_Cached yuv");
		return;
	}
	_item.HasValue = false;
	_indexes.push_back(_channelIndex);
	_widths.push_back(_width);
	_heights.push_back(_height);
	_ives.push_back(_item.IveBuffer);
	_param = "{\"Detect\":{\"DetectRegion\":[],\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}";
#endif // !_WIN32
	_params.resize(1);
	_timeStamps.resize(1);
	_result.resize(4 * 1024 * 1024);
}

DetectChannel::~DetectChannel()
{
#ifndef _WIN32
	delete[] _item.YuvTempBuffer;
	HI_MPI_SYS_MmzFree(_item.Yuv_phy_addr, reinterpret_cast<HI_VOID*>(_item.YuvBuffer));
	HI_MPI_SYS_MmzFree(_item.Ive_phy_addr, reinterpret_cast<HI_VOID*>(_item.IveBuffer));
#endif // !_WIN32
}

bool DetectChannel::IsBusy() 
{ 
	return _item.HasValue||!_inited || !_recogn->Inited();
}

void DetectChannel::HandleYUV(unsigned char* yuv, int width, int height, int packetIndex)
{
	memcpy(_item.YuvTempBuffer, yuv, _item.YuvSize);
	_item.PacketIndex = packetIndex;
	_item.HasValue = true;
}

bool DetectChannel::YuvToIve()
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
	yuv_image_list.au64PhyAddr[0] = _item.Yuv_phy_addr;
	yuv_image_list.au64PhyAddr[1] = yuv_image_list.au64PhyAddr[0] + _width * _height;
	yuv_image_list.au32Stride[0] = yuv_image_list.u32Width;
	yuv_image_list.au32Stride[1] = yuv_image_list.u32Width;

	yuv_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(_item.YuvBuffer);
	yuv_image_list.au64VirAddr[1] = yuv_image_list.au64VirAddr[0] + _width * _height;

	bgr_image_list.enType = IVE_IMAGE_TYPE_U8C3_PLANAR;
	bgr_image_list.u32Height = _height;
	bgr_image_list.u32Width = _width;
	bgr_image_list.au64PhyAddr[0] = _item.Ive_phy_addr;
	bgr_image_list.au64PhyAddr[1] = bgr_image_list.au64PhyAddr[0] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64PhyAddr[2] = bgr_image_list.au64PhyAddr[1] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(_item.IveBuffer);
	bgr_image_list.au64VirAddr[1] = bgr_image_list.au64VirAddr[0] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64VirAddr[2] = bgr_image_list.au64VirAddr[1] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au32Stride[0] = bgr_image_list.u32Width;
	bgr_image_list.au32Stride[1] = bgr_image_list.u32Width;
	bgr_image_list.au32Stride[2] = bgr_image_list.u32Width;

	hi_s32_ret = HI_MPI_IVE_CSC(&ive_handle, &yuv_image_list, &bgr_image_list, &ive_csc_ctrl, HI_TRUE);
	if (HI_SUCCESS != hi_s32_ret) {
		LogPool::Information("HI_MPI_IVE_CSC", hi_s32_ret);
		return false;
	}
	HI_BOOL ive_finish = HI_FALSE;
	hi_s32_ret = HI_SUCCESS;
	do {
		hi_s32_ret = HI_MPI_IVE_Query(ive_handle, &ive_finish, HI_TRUE);
	} while (HI_ERR_IVE_QUERY_TIMEOUT == hi_s32_ret);

	if (HI_SUCCESS != hi_s32_ret) {
		LogPool::Information("HI_MPI_IVE_Query", hi_s32_ret);
		return false;
	}
	//_yuvHandler->HandleFrame(_item.YuvBuffer, 1920, 1080, _item.PacketIndex);
	//_bgrHandler->HandleFrame(_item.IveBuffer, 1920, 1080, _item.PacketIndex);

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

void DetectChannel::GetRecognItems(vector<RecognItem>* items, const JsonDeserialization& jd, const string& key)
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
			item.ChannelIndex = _channelIndex;
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
		int result = SeemmoSDK::seemmo_thread_init(1, _channelIndex % 2, 1);
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
		if (_item.HasValue)
		{
			long long detectTimeStamp = DateTime::UtcNowTimeStamp();
			memcpy(_item.YuvBuffer, _item.YuvTempBuffer, _item.YuvSize);
			_params[0] = _param.c_str();
			_timeStamps[0] = _item.PacketIndex;
			_item.HasValue = false;
			if (YuvToIve())
			{
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
				if (result == 0)
				{
					JsonDeserialization detectJd(_result.data());
					map<string, DetectItem> detectItems;
					GetDetecItems(&detectItems, detectJd, "Vehicles");
					GetDetecItems(&detectItems, detectJd, "Bikes");
					GetDetecItems(&detectItems, detectJd, "Pedestrains");

					_detector->HandleDetect(&detectItems, detectTimeStamp,&_param, _ives[0], _timeStamps[0]);

					vector<RecognItem> recognItems;
					GetRecognItems(&recognItems, detectJd, "Vehicles");
					GetRecognItems(&recognItems, detectJd, "Bikes");
					GetRecognItems(&recognItems, detectJd, "Pedestrains");
					if (!recognItems.empty())
					{
						_recogn->PushItems(recognItems);
					}
				}
				//LogPool::Debug("detect", _indexes[0], _timeStamps[0],DateTime::UtcNowTimeStamp() - detectTimeStamp);
			}
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

