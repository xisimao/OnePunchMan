#include "DetectChannel.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;
using namespace TerribleTornado;

const int DetectChannel::SleepTime=10;
const int DetectChannel::ItemCount =1;
const string DetectChannel::IOTopic("IO");

DetectChannel::DetectChannel(int detectIndex, int width, int height, MqttChannel* mqtt, const vector<LaneDetector*>& lanes,RecognChannel* regon)
	:ThreadObject("detect"), _detectIndex(detectIndex), _width(width), _height(height)
	,_mqtt(mqtt), _yuvTopic(StringEx::Combine("YUV",detectIndex)),_lanes(lanes), _recogn(regon)
{
	for (int i = 0; i < ItemCount; ++i)
	{
		FrameItem item;
		item.YuvSize = static_cast<int>(_width * _height * 1.5);
		if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&item.Yuv_phy_addr),
			reinterpret_cast<HI_VOID**>(&item.YuvBuffer),
			"yuv_buffer",
			NULL,
			item.YuvSize) != HI_SUCCESS) {
			LogPool::Information("HI_MPI_SYS_MmzAlloc_Cached yuv");
			return;
		}
		item.YuvTempBuffer = new uint8_t[item.YuvSize];

		item.BgrSize = _width * _height * 3;
		if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&item.Bgr_phy_addr),
			reinterpret_cast<HI_VOID**>(&item.BgrBuffer),
			"bgr_buffer",
			NULL,
			item.BgrSize) != HI_SUCCESS) {
			LogPool::Information("HI_MPI_SYS_MmzAlloc_Cached yuv");
			return;
		}

		item.HasValue = false;
		item.CurrentPacketIndex = 0;
		item.LastPacketIndex = 0;
		item.PacketSpan = 0;
		_items.push_back(item);
		_widths.push_back(_width);
		_heights.push_back(_height);
		_params.push_back("{\"Detect\":{\"DetectRegion\":[],\"IsDet\":true,\"MaxCarWidth\":10,\"MinCarWidth\":10,\"Mode\":0,\"Threshold\":20,\"Version\":1001}}");
	}
	_bgrs.resize(ItemCount);
	_yuvs.resize(ItemCount);
	_indexes.resize(ItemCount);
	_timeStamps.resize(ItemCount);
	_result.resize(4 * 1024 * 1024);

}

DetectChannel::~DetectChannel()
{
	for (int i = 0; i < ItemCount; ++i)
	{
		delete[] _items[i].YuvTempBuffer;
		HI_MPI_SYS_MmzFree(_items[i].Yuv_phy_addr, reinterpret_cast<HI_VOID*>(_items[i].YuvBuffer));
		HI_MPI_SYS_MmzFree(_items[i].Bgr_phy_addr, reinterpret_cast<HI_VOID*>(_items[i].BgrBuffer));
	}
}

bool DetectChannel::IsBusy(int index) 
{ 
	return index<0||index>=ItemCount||_items[index].HasValue||!_recogn->Inited();
}

void DetectChannel::HandleYUV(unsigned char* yuv, int width, int height, int packetIndex,int index)
{
	if (index >= 0 && index < ItemCount)
	{
		FrameItem& item = _items[index];
		memcpy(item.YuvTempBuffer, yuv, item.YuvSize);
		item.CurrentPacketIndex = packetIndex;
		item.HasValue = true;
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
		int result = SeemmoSDK::seemmo_thread_init(1, _detectIndex % 2, ItemCount);
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

	vector<string> recognGuids;
	map<string, DetectItem> detectItems;
	while (!_cancelled)
	{
		long long detectTimeStamp = DateTime::TimeStamp();
		int itemIndex = 0;
		for (int i = 0; i < ItemCount; ++i)
		{
			FrameItem& item = _items[i];
			if (item.HasValue)
			{
				memcpy(item.YuvBuffer, item.YuvTempBuffer,item.YuvSize);
				item.PacketSpan = item.CurrentPacketIndex - item.LastPacketIndex;
				item.LastPacketIndex = item.CurrentPacketIndex;
				item.HasValue = false;
				if (YuvToBgr(item))
				{
					_bgrs[itemIndex] = item.BgrBuffer;
					//_yuvs[itemIndex] = item.YuvBuffer;
					_indexes[itemIndex] = _detectIndex * ItemCount + i;
					_timeStamps[itemIndex] = detectTimeStamp;
					itemIndex += 1;
				}
			}
		}
		if (itemIndex == 0)
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
		else
		{
			int32_t size = static_cast<int32_t>(_result.size());
			int result = SeemmoSDK::seemmo_video_pvc(static_cast<int32_t>(itemIndex),
				_indexes.data(),
				_timeStamps.data(),
				const_cast<const std::uint8_t**>(_bgrs.data()),
				_heights.data(),
				_widths.data(),
				_params.data(),
				_result.data(),
				&size,
				0);
			if (result == 0)
			{
	/*			for (int i = 0; i < itemIndex; ++i)
				{
					_mqtt->Send(_yuvTopic, _yuvs[i],_width*_height*1.5, true);
				}*/
				JsonDeserialization detectJd(_result.data());
				detectItems.clear();
				GetDetecItems(&detectItems, detectJd, "Vehicles");
				GetDetecItems(&detectItems, detectJd, "Bikes");
				GetDetecItems(&detectItems, detectJd, "Pedestrains");
				HandleDetect(detectItems, detectTimeStamp);

				recognGuids.clear();
				GetRecognGuids(&recognGuids, detectJd, "FilterResults", "Vehicles");
				GetRecognGuids(&recognGuids, detectJd, "FilterResults", "Bikes");
				GetRecognGuids(&recognGuids, detectJd, "FilterResults", "Pedestrains");
				_recogn->PushGuids(recognGuids);
			}
			LogPool::Debug("detect span", _indexes[0], DateTime::TimeStamp() - detectTimeStamp, detectItems.size(), recognGuids.size(), itemIndex, _items[_indexes[0] % ItemCount].LastPacketIndex);
		}
	}

	if (SeemmoSDK::seemmo_thread_uninit != NULL)
	{
		SeemmoSDK::seemmo_thread_uninit();
	}
}

bool DetectChannel::YuvToBgr(FrameItem& item)
{
	IVE_IMAGE_S yuv_image_list;
	IVE_IMAGE_S bgr_image_list;

	IVE_HANDLE ive_handle;
	IVE_CSC_CTRL_S ive_csc_ctrl = { IVE_CSC_MODE_PIC_BT709_YUV2RGB };
	int hi_s32_ret = HI_SUCCESS;

	yuv_image_list.enType = IVE_IMAGE_TYPE_YUV420SP;
	yuv_image_list.u32Height = _height;
	yuv_image_list.u32Width = _width;
	yuv_image_list.au64PhyAddr[0] = item.Yuv_phy_addr;
	yuv_image_list.au64PhyAddr[1] = yuv_image_list.au64PhyAddr[0] + _width * _height;
	yuv_image_list.au32Stride[0] = yuv_image_list.u32Width;
	yuv_image_list.au32Stride[1] = yuv_image_list.u32Width;

	yuv_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(item.YuvBuffer);
	yuv_image_list.au64VirAddr[1] = yuv_image_list.au64VirAddr[0] + _width * _height;

	bgr_image_list.enType = IVE_IMAGE_TYPE_U8C3_PLANAR;
	bgr_image_list.u32Height = _height;
	bgr_image_list.u32Width = _width;
	bgr_image_list.au64PhyAddr[0] = item.Bgr_phy_addr;
	bgr_image_list.au64PhyAddr[1] = bgr_image_list.au64PhyAddr[0] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64PhyAddr[2] = bgr_image_list.au64PhyAddr[1] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(item.BgrBuffer);
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
	return true;
}

void DetectChannel::GetRecognGuids(vector<string>* guids, const JsonDeserialization& jd, const string& key1, const string& key2)
{
	int itemIndex = 0;
	while (true)
	{
		string id = jd.Get<string>(StringEx::Combine(key1, ":0:", key2, ":", itemIndex, ":GUID"));
		if (id.empty())
		{
			break;
		}
		guids->push_back(id);
		itemIndex += 1;
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
			items->insert(pair<string, DetectItem>(id, DetectItem(Saitama::Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]), type)));
		}
		itemIndex += 1;
	}
}

void DetectChannel::HandleDetect(const map<string, DetectItem>& detectItems,long long timeStamp)
{
	if (detectItems.empty())
	{
		return;
	}
	string lanesJson;
	unique_lock<mutex> lck(_laneMutex);
	for (unsigned int laneIndex = 0; laneIndex < _lanes.size(); ++laneIndex)
	{
		IOStatus status = _lanes[laneIndex]->Detect(detectItems, timeStamp);
		if (status != IOStatus::UnChanged)
		{
			string laneJson;
			JsonSerialization::Serialize(&laneJson, "laneIndex", laneIndex);
			JsonSerialization::Serialize(&laneJson, "status", (int)status);
			JsonSerialization::Serialize(&laneJson, "type", (int)DetectType::Car);
			JsonSerialization::SerializeItem(&lanesJson, laneJson);
		}
	}
	lck.unlock();
	if (!lanesJson.empty())
	{
		string channelJson;
		JsonSerialization::SerializeJsons(&channelJson, "laneStatus", lanesJson);

		string channelsJson;
		JsonSerialization::SerializeItem(&channelsJson, channelJson);

		string ioJson;
		JsonSerialization::Serialize(&ioJson, "timestamp", timeStamp);
		JsonSerialization::SerializeJson(&ioJson, "detail", channelsJson);

		_mqtt->Send(IOTopic, ioJson, true);
	}
}


