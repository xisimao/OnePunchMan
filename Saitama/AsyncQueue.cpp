//#include "VideoChannel.h"
//
//using namespace std;
//using namespace Saitama;
//
//VideoChannel::VideoChannel()
//	:ThreadObject("video"), Id(), Index()
//{
//
//}
//
//VideoChannel::~VideoChannel()
//{
//	ClearLanes();
//}
//
//void VideoChannel::StartCore()
//{
//	while (!Cancelled())
//	{
//		if (_datas.empty())
//		{
//			this_thread::sleep_for(chrono::milliseconds(100));
//		}
//		else
//		{
//			long long timeStamp = DateTime::Now().Milliseconds();
//
//			unique_lock<mutex> dataLock(_dataMutex);
//			string data = _datas.front();
//			_datas.pop();
//			dataLock.unlock();
//
//			vector<DetectItem> vehicleItems = GetDetectItems(data, "Vehicles", timeStamp);
//			vector<DetectItem> bikeItems = GetDetectItems(data, "Bikes", timeStamp);
//			vector<DetectItem> pedestrainItems = GetDetectItems(data, "Pedestrains", timeStamp);
//
//			lock_guard<mutex> laneLock(_laneMutex);
//			for (vector<Lane*>::iterator lit = _lanes.begin(); lit != _lanes.end(); ++lit)
//			{
//				bool status = false;
//				for (vector<DetectItem>::iterator dit = vehicleItems.begin(); dit != vehicleItems.end(); ++dit)
//				{
//					if ((*lit)->DetectVehicle(*dit))
//					{
//						status = true;
//					}
//				}
//
//				for (vector<DetectItem>::iterator bit = bikeItems.begin(); bit != bikeItems.end(); ++bit)
//				{
//					if ((*lit)->DetectBike(*bit))
//					{
//						status = true;
//					}
//				}
//
//				for (vector<DetectItem>::iterator pit = pedestrainItems.begin(); pit != pedestrainItems.end(); ++pit)
//				{
//					if ((*lit)->DetectPedestrain(*pit))
//					{
//						status = true;
//					}
//				}
//
//				if ((*lit)->Status != status)
//				{
//					IOChangedEventArgs item(Id, Index, (*lit)->Id(), (*lit)->Index(), status);
//					IOChanged.Notice(&item);
//					(*lit)->Status = status;
//				}
//			}
//		}
//	}
//}
//
//void VideoChannel::UpdateLanes(const vector<Lane*>& lanes)
//{
//	ClearLanes();
//	lock_guard<mutex> lck(_laneMutex);
//	_lanes.assign(lanes.begin(), lanes.end());
//}
//
//void VideoChannel::Push(const string& data)
//{
//	lock_guard<mutex> lck(_dataMutex);
//	_datas.push(data);
//}
//
//vector<LaneItem> VideoChannel::Collect()
//{
//	vector<LaneItem> lanes;
//	lock_guard<mutex> lck(_laneMutex);
//	for (vector<Lane*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
//	{
//		lanes.push_back((*it)->Collect());
//	}
//	return lanes;
//}
//
//void VideoChannel::ClearLanes()
//{
//	lock_guard<mutex> lck(_laneMutex);
//	for (vector<Lane*>::iterator it = _lanes.begin(); it != _lanes.end(); ++it)
//	{
//		delete (*it);
//	}
//	_lanes.clear();
//}
//
//vector<DetectItem> VideoChannel::GetDetectItems(const std::string& json, const std::string& key, long long timeStamp)
//{
//	vector<string> values;
//	JsonFormatter::Deserialize(json, key, &values);
//	vector<DetectItem> items;
//	for (vector<string>::iterator it = values.begin(); it != values.end(); ++it)
//	{
//		string id;
//		JsonFormatter::Deserialize(*it, "GUID", &id);
//		int type;
//		JsonFormatter::Deserialize(*it, "Type", &type);
//		string detect;
//		JsonFormatter::Deserialize(*it, "Detect", &detect);
//		string body;
//		JsonFormatter::Deserialize(detect, "Body", &body);
//		vector<int> datas;
//		JsonFormatter::Deserialize(body, "Rect", &datas);
//		items.push_back(DetectItem(id, timeStamp, type, Rectangle(datas[0], datas[1], datas[2], datas[3])));
//	}
//	return items;
//
//}
//
