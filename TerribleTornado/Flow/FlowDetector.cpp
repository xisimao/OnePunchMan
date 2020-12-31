#include "FlowDetector.h"

using namespace std;
using namespace OnePunchMan;

const string FlowDetector::IOTopic("IO");
const string FlowDetector::FlowTopic("Flow");
const string FlowDetector::VideoStructTopic("VideoStruct");
const int FlowDetector::ReportMaxSpan = 60 * 1000;
const int FlowDetector::DeleteSpan = 60 * 1000;
int FlowDetector::QueueMinDistance = 200;

FlowDetector::FlowDetector(int width, int height,SocketMaid* maid, MqttChannel* mqtt, DataChannel* data)
	:TrafficDetector(width, height, mqtt), _maid(maid),_data(data)
	, _taskId(0)
	, _lastFrameTimeStamp(0), _currentMinuteTimeStamp(0), _nextMinuteTimeStamp(0)
	, _detectImage(width,height,false),_recognImage(width,height,false)
	, _outputReport(false), _currentReportMinute(0), _outputRecogn(false)
{

}

void FlowDetector::Init(const JsonDeserialization& jd)
{
	QueueMinDistance = jd.Get<int>("Flow:QueueMinDistance");
	LogPool::Information(LogEvent::Event, "QueueMinDistance", QueueMinDistance, "px");
}

void FlowDetector::UpdateChannel(const unsigned char taskId, const TrafficChannel& channel)
{
	string log(StringEx::Combine("��ʼ���������,ͨ�����:", channel.ChannelIndex, "������:", _taskId));

	vector<FlowData> lanes;
	for (vector<FlowLane>::const_iterator it = channel.FlowLanes.begin(); it != channel.FlowLanes.end(); ++it)
	{
		FlowData laneCache;
		Line stopLine = Line::FromJson(it->StopLine);
		Line queueDetectLine = Line::FromJson(it->QueueDetectLine);
		BrokenLine laneLine1 = BrokenLine::FromJson(it->LaneLine1);
		BrokenLine laneLine2 = BrokenLine::FromJson(it->LaneLine2);
		laneCache.FlowRegion = Polygon::FromJson(it->FlowRegion);
		laneCache.QueueRegion = Polygon::FromJson(it->QueueRegion);
		if (stopLine.Empty()
			|| queueDetectLine.Empty()
			|| laneCache.FlowRegion.Empty()
			|| laneCache.QueueRegion.Empty())
		{
			log.append(StringEx::Combine("ͨ��:",it->ChannelIndex,"����:",it->LaneIndex,"��ʼ��ʧ��"));
		}
		else
		{
			double pixels = (laneLine1.Points().front().Distance(laneLine2.Points().front())+ laneLine1.Points().back().Distance(laneLine2.Points().back()))/2;
			laneCache.LaneIndex = it->LaneIndex;
			laneCache.LaneId = it->LaneId;
			laneCache.LaneName = it->LaneName;
			laneCache.IOIp = it->IOIp;
			laneCache.IOIndex = it->IOIndex;
			laneCache.Direction = it->Direction;
			laneCache.ReportProperties = channel.ReportProperties;
			laneCache.MeterPerPixel = channel.LaneWidth / pixels;
			laneCache.LaneLength = stopLine.Middle().Distance(queueDetectLine.Middle())*laneCache.MeterPerPixel;
			laneCache.StopPoint = stopLine.Middle();
			laneCache.FreeSpeed = channel.FreeSpeed;
			lanes.push_back(laneCache);
		
			log.append(StringEx::Combine("ͨ��:", it->ChannelIndex, "����:", it->LaneIndex, "��ʼ���ɹ�","��������:",laneCache.FlowRegion.ToJson(),"�Ŷ�����:",laneCache.QueueRegion.ToJson(),"�������:", laneCache.MeterPerPixel,"m/px","�Ŷ����򳤶�:", laneCache.LaneLength));
		}
	}

	lock_guard<mutex> detectLock(_laneMutex);
	_taskId = taskId;
	_laneCaches.assign(lanes.begin(), lanes.end());
	_channelUrl = channel.ChannelUrl;

	_lanesInited = !_laneCaches.empty() && _laneCaches.size() == channel.FlowLanes.size();
	_channelIndex = channel.ChannelIndex;

	_reportCaches.clear();
	_vehicleReportCaches.clear();
	_bikeReportCaches.clear();
	_pedestrainReportCaches.clear();
	_outputReport = channel.OutputReport;
	_outputRecogn = channel.OutputRecogn;
	_currentReportMinute = 0;
	DateTime date;
	if (channel.OutputReport)
	{
		date = DateTime::ParseTimeStamp(0);
	}
	else
	{
		date = DateTime::Now();
	}
	_currentMinuteTimeStamp = DateTime(date.Year(), date.Month(), date.Day(), date.Hour(), date.Minute(), 0).TimeStamp();
	_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
	log.append(StringEx::Combine("ʱ���:", DateTime::ParseTimeStamp(_currentMinuteTimeStamp).ToString(), "->", DateTime::ParseTimeStamp(_nextMinuteTimeStamp).ToString()));
	if (_lanesInited)
	{
		LogPool::Information(LogEvent::Flow, log);	
	}
	else
	{
		log.append("û���κγ�������");
		LogPool::Warning(LogEvent::Flow, log);
	}
}

void FlowDetector::ClearChannel()
{
	lock_guard<mutex> detectLock(_laneMutex);
	_taskId = 0;
	_laneCaches.clear();
	_channelUrl = string();
	_lanesInited = false;
}

void FlowDetector::ResetTimeRange()
{
	_currentMinuteTimeStamp = 0;
	_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
}

void FlowDetector::GetReportJson(string* json)
{
	lock_guard<mutex> lock(_laneMutex);
	string flowsJson;
	for (vector<FlowData>::iterator it = _reportCaches.begin(); it != _reportCaches.end(); ++it)
	{	
		JsonSerialization::AddClassItem(&flowsJson, it->ToReportJson());
	}
	string vehiclesJson;
	for (vector<VehicleData>::iterator it = _vehicleReportCaches.begin(); it != _vehicleReportCaches.end(); ++it)
	{
		JsonSerialization::AddClassItem(&vehiclesJson, it->ToJson());
	}
	string bikesJson;
	for (vector<BikeData>::iterator it = _bikeReportCaches.begin(); it != _bikeReportCaches.end(); ++it)
	{
		JsonSerialization::AddClassItem(&bikesJson, it->ToJson());
	}
	string pedestrainsJson;
	for (vector<PedestrainData>::iterator it = _pedestrainReportCaches.begin(); it != _pedestrainReportCaches.end(); ++it)
	{
		JsonSerialization::AddClassItem(&pedestrainsJson, it->ToJson());
	}
	JsonSerialization::SerializeClass(json, "flows", flowsJson);
	JsonSerialization::SerializeClass(json, "vehicles", vehiclesJson);
	JsonSerialization::SerializeClass(json, "bikes", bikesJson);
	JsonSerialization::SerializeClass(json, "pedestrains", pedestrainsJson);
}

FlowData FlowDetector::CalculateMinuteFlow(FlowData* laneCache)
{
	FlowData data;
	data.ChannelUrl = _channelUrl;
	data.LaneId = laneCache->LaneId;
	data.LaneName = laneCache->LaneName;
	data.Direction = laneCache->Direction;
	data.ReportProperties = laneCache->ReportProperties;

	data.Minute = _currentReportMinute;
	data.TimeStamp = _currentMinuteTimeStamp;

	data.Persons = laneCache->Persons;
	data.Bikes = laneCache->Bikes;
	data.Motorcycles = laneCache->Motorcycles;
	data.Cars = laneCache->Cars;
	data.Tricycles = laneCache->Tricycles;
	data.Buss = laneCache->Buss;
	data.Vans = laneCache->Vans;
	data.Trucks = laneCache->Trucks;


	//�ƶ��ܾ���(km)
	double totalDistance = (laneCache->TotalDistance * laneCache->MeterPerPixel / 1000.0);
	//�г���ʱ��(h)
	double totalTime = static_cast<double>(laneCache->TotalTime) / 3600000.0;
	//ƽ���ٶ�(km/h)
	data.Speed = laneCache->TotalTime == 0 ? 0 : totalDistance / totalTime;

	//����������
	int vehicles = data.Cars + data.Tricycles + data.Buss + data.Vans + data.Trucks;
	//��ͷʱ��(sec)
	data.HeadDistance = vehicles > 1 ? static_cast<double>(laneCache->TotalSpan) / static_cast<double>(vehicles - 1) / 1000.0 : 0;
	
	//��ͷ���(m)
	data.HeadSpace = data.Speed * 1000 * data.HeadDistance / 3600.0;
	
	//ʱ��ռ����(%)
	data.TimeOccupancy = static_cast<double>(laneCache->TotalInTime) / 60000.0 * 100;
	
	//�������г�ʱ��(h)
	double freeTime = totalDistance / laneCache->FreeSpeed;
	//��������ʱ��(s)
	double delayTime = (totalTime - freeTime)*3600;
	//��ͨ״̬
	if (delayTime<55)
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Good);
	}
	else if (delayTime>=55&& delayTime<100)
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Warning);
	}
	else if (delayTime>=100&& delayTime<145)
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Bad);
	}
	else
	{
		data.TrafficStatus = static_cast<int>(TrafficStatus::Dead);
	}

	//�Ŷӳ���(m)
	data.QueueLength = laneCache->MaxQueueLength;

	//�ռ�ռ����(%)
	data.SpaceOccupancy = (laneCache->LaneLength == 0 || laneCache->CountQueueLength == 0) ? 0 : laneCache->TotalQueueLength / static_cast<double>(laneCache->LaneLength) / static_cast<double>(laneCache->CountQueueLength) * 100;

	LogPool::Information(LogEvent::Flow,"�������� ͨ�����:", _channelIndex, "�������:", laneCache->LaneIndex,"��ǰʱ��:",DateTime::ParseTimeStamp(_currentMinuteTimeStamp).ToString()
		, "�γ�:", data.Cars, "����:", data.Trucks, "�ͳ�:", data.Buss, "�����:", data.Vans, "���ֳ�:", data.Tricycles, "Ħ�г�:", data.Motorcycles, "���г�:", data.Bikes, "����:", data.Persons
		, "ƽ���ٶ�(km/h):", data.Speed, "���ƶ�����(px):", laneCache->TotalDistance, "ÿ���ش������(m/px):", laneCache->MeterPerPixel, "���г�ʱ��(ms)", laneCache->TotalTime
		, "��ͷʱ��(sec):", data.HeadDistance, "������������ʱ���ĺ�(ms):", laneCache->TotalSpan, "����������:", vehicles
		, "��ͷ���(m):", data.HeadSpace
		, "ʱ��ռ����(%):", data.TimeOccupancy, "����ռ����ʱ��(ms):", laneCache->TotalInTime
		, "�Ŷӳ���(m):", data.QueueLength
		, "�ռ�ռ����(%):", data.SpaceOccupancy, "���Ŷӳ���(m):", laneCache->TotalQueueLength, "��������(m):", laneCache->LaneLength, "���Ŷӳ��ȼ�������:", laneCache->CountQueueLength
		, "��ͨ״̬:", data.TrafficStatus, "�������ٶ�(km/h)", data.FreeSpeed);

	laneCache->Persons = 0;
	laneCache->Bikes = 0;
	laneCache->Motorcycles = 0;
	laneCache->Tricycles = 0;
	laneCache->Trucks = 0;
	laneCache->Vans = 0;
	laneCache->Cars = 0;
	laneCache->Buss = 0;

	laneCache->TotalDistance = 0.0;
	laneCache->TotalTime = 0;

	laneCache->TotalInTime = 0;

	laneCache->LastInRegion = 0;
	laneCache->TotalSpan = 0;

	laneCache->MaxQueueLength = 0;
	laneCache->TotalQueueLength = 0;
	laneCache->CountQueueLength = 0;
	laneCache->CurrentQueueLength = 0;

	return data;
}

void FlowDetector::HandleDetect(map<string, DetectItem>* detectItems, long long timeStamp, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan)
{
	if (_taskId != taskId)
	{
		return;
	}
	lock_guard<mutex> detectLock(_laneMutex);
	//�����ⱨ��ʱ����ʱ���
	if (_outputReport)
	{
		timeStamp = frameIndex * frameSpan;
	}
	//������һ����
	if (timeStamp > _nextMinuteTimeStamp)
	{
		_currentReportMinute += 1;
		string flowLanesJson;
		for (unsigned int i = 0; i < _laneCaches.size(); ++i)
		{
			FlowData& laneCache = _laneCaches[i];
			FlowData data=CalculateMinuteFlow(&laneCache);	
		
			if (laneCache.ReportProperties == TrafficChannel::AllPropertiesFlag)
			{			
				JsonSerialization::AddClassItem(&flowLanesJson, data.ToMessageJson());
			}
			else
			{
				if (_data != NULL)
				{
					_data->PushFlowData(data);
				}
			}

			if (_outputReport)
			{	
				_reportCaches.push_back(data);
			}
		
		}

		if (!flowLanesJson.empty()
			&& _mqtt != NULL
			&& !_outputReport
			&& (timeStamp - _nextMinuteTimeStamp) < ReportMaxSpan)
		{
		
			_mqtt->Send(FlowTopic, flowLanesJson);
		}

		//�������Ϊ�÷��ӽ���,��ǰ֡�յ������ݽ��㵽��һ����
		DateTime currentTime=DateTime::ParseTimeStamp(timeStamp);
		DateTime currentTimePoint(currentTime.Year(), currentTime.Month(), currentTime.Day(), currentTime.Hour(), currentTime.Minute(), 0);
		_currentMinuteTimeStamp = currentTimePoint.TimeStamp();
		_lastFrameTimeStamp = _currentMinuteTimeStamp;
		_nextMinuteTimeStamp = _currentMinuteTimeStamp + 60 * 1000;
	}

	//���㵱ǰ֡
	string ioLanesJson;
	for (unsigned int i = 0; i < _laneCaches.size(); ++i)
	{
		FlowData& laneCache = _laneCaches[i];
		//ɾ����ʱ����
		for (map<string, FlowDetectCache>::iterator it = laneCache.Items.begin(); it != laneCache.Items.end();)
		{
			if (timeStamp - it->second.LastTimeStamp > DeleteSpan)
			{
				if (it->second.TotalTime == 0)
				{
					LogPool::Information(LogEvent::Flow, "���������", it->first, "ʱ��Ϊ0");

				}
				else
				{
					LogPool::Information(LogEvent::Flow, "���������", it->first, "�ٶ�(km/h):", it->second.TotalDistance * it->second.MeterPerPixel / 1000.0 / (static_cast<double>(it->second.TotalTime) / 3600000.0), it->second.TotalDistance, it->second.TotalTime);
				}
				laneCache.Items.erase(it++);		
			}
			else
			{
				++it;
			}
		}
		list<double> distances;
		bool ioStatus = false;
		for (map<string, DetectItem>::iterator dit = detectItems->begin(); dit != detectItems->end(); ++dit)
		{
			if (dit->second.Status != DetectStatus::Out)
			{
				continue;
			}
			if (laneCache.FlowRegion.Contains(dit->second.Region.HitPoint()))
			{
				ioStatus = true;
				map<string, FlowDetectCache>::iterator mit = laneCache.Items.find(dit->first);
				//������³�����������ͳ�ͷʱ��
				//����=������
				//��ͷʱ��=���н��������ʱ����ƽ��ֵ
				if (mit == laneCache.Items.end())
				{
					dit->second.Status = DetectStatus::New;
					if (dit->second.Type == DetectType::Car)
					{
						laneCache.Cars += 1;
					}
					else if (dit->second.Type == DetectType::Tricycle)
					{
						laneCache.Tricycles += 1;
					}
					else if (dit->second.Type == DetectType::Bus)
					{
						laneCache.Buss += 1;
					}
					else if (dit->second.Type == DetectType::Van)
					{
						laneCache.Vans += 1;
					}
					else if (dit->second.Type == DetectType::Truck)
					{
						laneCache.Trucks += 1;
					}
					else if (dit->second.Type == DetectType::Bike)
					{
						laneCache.Bikes += 1;
					}
					else if (dit->second.Type == DetectType::Motobike)
					{
						laneCache.Motorcycles += 1;
					}
					else if (dit->second.Type == DetectType::Pedestrain)
					{
						laneCache.Persons += 1;
					}
					if (laneCache.LastInRegion != 0)
					{
						laneCache.TotalSpan += timeStamp - laneCache.LastInRegion;
					}
					laneCache.LastInRegion = timeStamp;
					FlowDetectCache detectCache;
					detectCache.LastHitPoint = dit->second.Region.HitPoint();
					detectCache.LastTimeStamp = timeStamp;
					detectCache.MeterPerPixel = laneCache.MeterPerPixel;
					laneCache.Items.insert(pair<string, FlowDetectCache>(dit->first, detectCache));
				}
				//������Ѿ���¼�ĳ������ƽ���ٶ�
				//ƽ���ٶ�=�ܾ���/��ʱ��
				//�ܾ���=���μ�⵽�ĵ�ľ���*ÿ�����ش��������
				//��ʱ��=���μ�⵽��ʱ���ʱ��
				else
				{
					dit->second.Status = DetectStatus::In;
					double distance = dit->second.Region.HitPoint().Distance(mit->second.LastHitPoint);
					long long time = timeStamp - _lastFrameTimeStamp;
					laneCache.TotalDistance += distance;
					laneCache.TotalTime += time;
					mit->second.TotalDistance += distance;
					mit->second.TotalTime += time;
					mit->second.LastHitPoint = dit->second.Region.HitPoint();
					mit->second.LastTimeStamp = timeStamp;
				}
			}

			if (laneCache.QueueRegion.Contains(dit->second.Region.HitPoint()))
			{
				dit->second.Distance = dit->second.Region.HitPoint().Distance(laneCache.StopPoint);
				AddOrderedList(&distances, dit->second.Distance);
			}
		}
		//�����Ŷӳ���
		laneCache.CurrentQueueLength = CalculateQueueLength(laneCache,distances);
		if (laneCache.CurrentQueueLength != 0)
		{
			laneCache.TotalQueueLength += laneCache.CurrentQueueLength;
			laneCache.CountQueueLength += 1;
			if (laneCache.CurrentQueueLength > laneCache.MaxQueueLength)
			{
				laneCache.MaxQueueLength = laneCache.CurrentQueueLength;
			}		
		}
	
		//�����һ���г�,����Ϊ����μ��Ϊֹ���г�
		//ʱ��ռ����=��ʱ��/һ����
		if (laneCache.IoStatus)
		{
			laneCache.TotalInTime += timeStamp - _lastFrameTimeStamp;
		}

		//io״̬�ı��ϱ�
		if (ioStatus != laneCache.IoStatus)
		{
			laneCache.IoStatus = ioStatus;
			string laneJson;
			JsonSerialization::SerializeValue(&laneJson, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&laneJson, "laneId", laneCache.LaneId);
			JsonSerialization::SerializeValue(&laneJson, "timeStamp", timeStamp);
			JsonSerialization::SerializeValue(&laneJson, "status", (int)ioStatus);
			JsonSerialization::AddClassItem(&ioLanesJson, laneJson);
			if (_maid != NULL && !laneCache.IOIp.empty() && laneCache.IOIndex != 0)
			{
				EndPoint ep(laneCache.IOIp, 24000);
				std::stringstream ss;
				ss << "|set|"
					<< setw(2) << setfill('0') << (laneCache.IOIndex - 1)
					<< ":"
					<< (int)ioStatus
					<< "|";
				SocketResult r = _maid->SendTcp(ep, ss.str());
				if (r != SocketResult::Success)
				{
					LogPool::Error(LogEvent::Flow, "IOת��������ʧ��", laneCache.IOIp);
				}
			}
			LogPool::Information(LogEvent::Flow, "IO���� ͨ�����:", _channelIndex, "�������:", laneCache.LaneIndex, "��ǰʱ��:", DateTime::ParseTimeStamp(timeStamp).ToString(), "IO״̬:", ioStatus);
		}
	}

	//����ʱ���
	_lastFrameTimeStamp = timeStamp;

	//��������
	if (!ioLanesJson.empty()
		&& _mqtt != NULL
		&& !_outputReport)
	{
		_mqtt->Send(IOTopic, ioLanesJson);
	}
}

void FlowDetector::FinishDetect(unsigned char taskId)
{
	if (_taskId != taskId)
	{
		return;
	}
	if (_outputReport)
	{
		lock_guard<mutex> lock(_laneMutex);
		_currentReportMinute += 1;
		for (unsigned int i = 0; i < _laneCaches.size(); ++i)
		{
			FlowData& cache = _laneCaches[i];
			FlowData reportCache=CalculateMinuteFlow(&cache);
			_reportCaches.push_back(reportCache);
		}
	}
}

void FlowDetector::HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, VehicleData* vehicle)
{
	if (_taskId != recognItem.TaskId)
	{
		return;
	}
	lock_guard<mutex> recognLock(_laneMutex);
	for (vector<FlowData>::iterator it = _laneCaches.begin(); it != _laneCaches.end(); ++it)
	{
		if (it->Items.find(recognItem.Guid) != it->Items.end())
		{
			vehicle->ChannelUrl = _channelUrl;
			vehicle->LaneId = it->LaneId;
			vehicle->TimeStamp = DateTime::NowTimeStamp();
			vehicle->Guid = recognItem.Guid;
			_recognImage.IveToBmp(iveBuffer, recognItem.Width, recognItem.Height, TrafficDirectory::GetRecognBmp(recognItem.Guid));
			if (_data != NULL)
			{
				_data->PushVehicleData(*vehicle);
			}

			string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", it->LaneId);
			DateTime time = DateTime::Now();
			DateTime adjustTime = DateTime(time.Year() + 1, time.Month(), time.Day(), time.Hour(), time.Minute(), time.Second(), time.Millisecond());
			JsonSerialization::SerializeValue(&json, "timeStamp", adjustTime.TimeStamp());
			JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Vehicle);
			JsonSerialization::SerializeValue(&json, "carType", vehicle->CarType);
			JsonSerialization::SerializeValue(&json, "carColor", vehicle->CarColor);
			JsonSerialization::SerializeValue(&json, "carBrand", vehicle->CarBrand);
			JsonSerialization::SerializeValue(&json, "plateType", vehicle->PlateType);
			JsonSerialization::SerializeValue(&json, "plateNumber", vehicle->PlateNumber);
			string image = _recognImage.IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height);
			JsonSerialization::SerializeValue(&json, "image", image);
			if (_outputRecogn)
			{
				VehicleData reportCache = *vehicle;
				reportCache.LaneId = it->LaneId;
				reportCache.LaneName = it->LaneName;
				reportCache.Direction = it->Direction;
				reportCache.Minute = recognItem.FrameIndex * recognItem.FrameSpan / 60000;
				reportCache.Second = recognItem.FrameIndex * recognItem.FrameSpan % 60000 / 1000;
				_vehicleReportCaches.push_back(reportCache);
			}
			else
			{
				if (_mqtt != NULL)
				{
					_mqtt->Send(VideoStructTopic, json);
				}
			}
			return;
		}
	}
	LogPool::Debug(LogEvent::Flow, "lost recogn,id:", recognItem.Guid, " type:", static_cast<int>(recognItem.Type), " frame index:", recognItem.FrameIndex);
}

void FlowDetector::HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, BikeData* bike)
{
	if (_taskId != recognItem.TaskId)
	{
		return;
	}
	lock_guard<mutex> recognLock(_laneMutex);
	for (vector<FlowData>::iterator it = _laneCaches.begin(); it != _laneCaches.end(); ++it)
	{
		if (it->Items.find(recognItem.Guid) != it->Items.end())
		{
			bike->ChannelUrl = _channelUrl;
			bike->LaneId = it->LaneId;
			bike->TimeStamp = DateTime::NowTimeStamp();
			bike->Guid = recognItem.Guid;
			_recognImage.IveToBmp(iveBuffer, recognItem.Width, recognItem.Height, TrafficDirectory::GetRecognBmp(recognItem.Guid));
			if (_data != NULL)
			{
				_data->PushBikeData(*bike);
			}

			string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", it->LaneId);
			DateTime time = DateTime::Now();
			DateTime adjustTime = DateTime(time.Year() + 1, time.Month(), time.Day(), time.Hour(), time.Minute(), time.Second(), time.Millisecond());
			JsonSerialization::SerializeValue(&json, "timeStamp", adjustTime.TimeStamp());
			JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Bike);
			JsonSerialization::SerializeValue(&json, "bikeType", bike->BikeType);
			string image = _recognImage.IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height);
			JsonSerialization::SerializeValue(&json, "image", image);
			if (_outputRecogn)
			{
				BikeData reportCache = *bike;
				reportCache.LaneId = it->LaneId;
				reportCache.LaneName = it->LaneName;
				reportCache.Direction = it->Direction;
				reportCache.Minute = recognItem.FrameIndex * recognItem.FrameSpan / 60000;
				reportCache.Second = recognItem.FrameIndex * recognItem.FrameSpan %60000/1000;
				_bikeReportCaches.push_back(reportCache);
			}
			else
			{
				if (_mqtt != NULL)
				{
					_mqtt->Send(VideoStructTopic, json);
				}
			}
			return;
		}
	}
	LogPool::Debug(LogEvent::Flow, "lost recogn,id:", recognItem.Guid, " type:", static_cast<int>(recognItem.Type), " frame index:", recognItem.FrameIndex);
}

void FlowDetector::HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, PedestrainData* pedestrain)
{
	if (_taskId != recognItem.TaskId)
	{
		return;
	}
	lock_guard<mutex> recognLock(_laneMutex);
	for (vector<FlowData>::iterator it = _laneCaches.begin(); it != _laneCaches.end(); ++it)
	{
		if (it->Items.find(recognItem.Guid) != it->Items.end())
		{
			pedestrain->ChannelUrl = _channelUrl;
			pedestrain->LaneId = it->LaneId;
			pedestrain->TimeStamp = DateTime::NowTimeStamp();
			pedestrain->Guid = recognItem.Guid;
			_recognImage.IveToBmp(iveBuffer, recognItem.Width, recognItem.Height, TrafficDirectory::GetRecognBmp(recognItem.Guid));
			if (_data != NULL)
			{
				_data->PushPedestrainData(*pedestrain);
			}

			string json;
			JsonSerialization::SerializeValue(&json, "channelUrl", _channelUrl);
			JsonSerialization::SerializeValue(&json, "laneId", it->LaneId);
			DateTime time = DateTime::Now();
			DateTime adjustTime = DateTime(time.Year() + 1, time.Month(), time.Day(), time.Hour(), time.Minute(), time.Second(), time.Millisecond());
			JsonSerialization::SerializeValue(&json, "timeStamp", adjustTime.TimeStamp());
			JsonSerialization::SerializeValue(&json, "videoStructType", (int)VideoStructType::Pedestrain);
			JsonSerialization::SerializeValue(&json, "sex", pedestrain->Sex);
			JsonSerialization::SerializeValue(&json, "age", pedestrain->Age);
			JsonSerialization::SerializeValue(&json, "upperColor", pedestrain->UpperColor);
			_recognImage.IveToBmp(iveBuffer, recognItem.Width, recognItem.Height, TrafficDirectory::GetRecognBmp(recognItem.Guid));
			string image=_recognImage.IveToJpgBase64(iveBuffer, recognItem.Width, recognItem.Height);
			JsonSerialization::SerializeValue(&json, "image", image);
			if (_outputRecogn)
			{
				PedestrainData reportCache = *pedestrain;
				reportCache.LaneId = it->LaneId;
				reportCache.LaneName = it->LaneName;
				reportCache.Direction = it->Direction;
				reportCache.Minute = recognItem.FrameIndex * recognItem.FrameSpan / 60000;
				reportCache.Second = recognItem.FrameIndex * recognItem.FrameSpan % 60000 / 1000;
				_pedestrainReportCaches.push_back(reportCache);
			}
			else
			{
				if (_mqtt != NULL)
				{
					_mqtt->Send(VideoStructTopic, json);
				}
			}
			return;
		}
	}
	LogPool::Debug(LogEvent::Flow, "lost recogn,id:", recognItem.Guid, " type:", static_cast<int>(recognItem.Type), " frame index:", recognItem.FrameIndex);
}

void FlowDetector::AddOrderedList(list<double>* distances, double distance)
{
	if (distances->empty())
	{
		distances->push_back(distance);
	}
	else
	{
		for (list<double>::iterator it = distances->begin(); it != distances->end(); ++it)
		{
			if (*it > distance)
			{
				distances->insert(it, distance);
				return;
			}
		}
		distances->push_back(distance);
	}
}

int FlowDetector::CalculateQueueLength(const FlowData& laneCache, const list<double>& distances)
{
	double maxDistance = 0.0;
	if (distances.size() > 1)
	{
		list<double>::const_iterator preCar = distances.begin();
		list<double>::const_iterator nextCar = ++distances.begin();
		while (preCar != distances.end() && nextCar != distances.end())
		{
			if (*nextCar-*preCar > QueueMinDistance)
			{
				break;
			}
			else
			{
				maxDistance = *nextCar;
			}
			preCar = nextCar;
			++nextCar;
		}
	}
	return static_cast<int>(maxDistance * laneCache.MeterPerPixel);
}

void FlowDetector::DrawDetect(const map<string, DetectItem>& detectItems, unsigned int frameIndex)
{
	cv::Mat image = cv::imread(TrafficDirectory::GetDataFrameJpg(_channelIndex,frameIndex));

	if (image.empty())
	{
		return;
	}
	//����
	for (unsigned int i = 0; i < _laneCaches.size(); ++i)
	{
		FlowData& cache = _laneCaches[i];
		//��������
		ImageDrawing::DrawPolygon(&image, cache.FlowRegion, cv::Scalar(0, 0, 255));
		//�Ŷ�����
		ImageDrawing::DrawPolygon(&image, cache.QueueRegion, cv::Scalar(0, 0, 200));
	}
	//�����
	for (map<string, DetectItem>::const_iterator it = detectItems.begin(); it != detectItems.end(); ++it)
	{
		cv::Scalar carScalar;
		//��ɫ�³�
		if (it->second.Status == DetectStatus::New)
		{
			carScalar = cv::Scalar(0, 255, 0);
		}
		//��ɫ������
		else if (it->second.Status == DetectStatus::In)
		{
			carScalar = cv::Scalar(0, 255, 255);
		}
		else
		{
			continue;
		}
		ImageDrawing::DrawRectangle(&image, it->second.Region, carScalar);
		ImageDrawing::DrawString(&image, StringEx::Combine(it->first.substr(it->first.size() - 4, 4), "-", static_cast<int>(it->second.Type)), it->second.Region.HitPoint(), carScalar);
	}
	_detectImage.BgrToJpgFile(image.data, _width, _height,TrafficDirectory::GetTempFrameJpg(_channelIndex,frameIndex));
}
