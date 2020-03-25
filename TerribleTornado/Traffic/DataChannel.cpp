#include "DataChannel.h"

using namespace std;
using namespace Saitama;

const string DataChannel::DetectTopic("aiservice/level_1_imageresult");
const string DataChannel::RecognizeTopic("aiservice/level_2_result");
const string DataChannel::FlowTopic("analysis2manager_crossingflows");
const string DataChannel::VideoTopic("analysis2manager_videostruct");
const string DataChannel::IOTopic("analysis2manager_deviceIO");

DataChannel::DataChannel(const std::string& ip, int port)
    :ThreadObject("data"), _mqtt(NULL),_ip(ip),_port(port)
{
    _lastMinute = DateTime::Now().Minute();
}

void DataChannel::Update(MqttReceivedEventArgs* e)
{
    if (e->Topic.compare(DetectTopic) == 0)
    {
        HandleDetect(e->Message);
    }
    else if (e->Topic.compare(RecognizeTopic) == 0)
    {
        HandleRecognize(e->Message);
    }
}

void DataChannel::StartCore()
{
    //初始化通道
    for (int i = 0; i < 8; ++i)
    {
        _channels.push_back(vector<LaneDetector*>());
    }
    FlowChannelData data;
    vector<FlowChannel> channels = data.GetList();
    for (vector<FlowChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        UpdateChannel(*it);
    }

    //初始化mqtt
    vector<string> topics;
    topics.push_back(DetectTopic);
    topics.push_back(RecognizeTopic);
    _mqtt = new MqttChannel(_ip, _port, topics);
    _mqtt->MqttReceived.Subscribe(this);
    _mqtt->Start();

    while (!Cancelled())
    {
        this_thread::sleep_for(chrono::seconds(1));

        //收集流量
        DateTime now = DateTime::Now();
        int currentMinute = now.Minute();
        if (currentMinute != _lastMinute)
        {
            _lastMinute = currentMinute;
            CollectFlow(now);
        }
    }

    _mqtt->Stop();
    delete _mqtt;
    for (vector<vector<LaneDetector*>>::iterator cit = _channels.begin(); cit != _channels.end(); ++cit)
    {
        for (vector<LaneDetector*>::iterator lit = cit->begin(); lit != cit->end(); ++lit)
        {
            delete (*lit);
        }
    }
}

void DataChannel::UpdateChannel(const FlowChannel& channel)
{
    if (channel.ChannelIndex >= 0 && static_cast<unsigned int>(channel.ChannelIndex) < _channels.size())
    {
        lock_guard<mutex> lck(_channelMutex);
        for (vector<LaneDetector*>::iterator it = _channels[channel.ChannelIndex].begin(); it != _channels[channel.ChannelIndex].end(); ++it)
        {
            delete (*it);
        }
        _channels[channel.ChannelIndex].clear();
        for (vector<Lane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
        {
            _channels[channel.ChannelIndex].push_back(new LaneDetector(StringEx::Combine(channel.ChannelUrl,"_", lit->LaneId),*lit));
        }
    }
}

void DataChannel::DeserializeDetectItems(map<string, DetectItem>* items,const JsonDeserialization& jd, const string& key, long long timeStamp)
{
    int itemIndex = 0;
    while (true)
    {
        string id = jd.Get<string>(StringEx::Combine("ImageResults:0:",key, ":", itemIndex, ":GUID"));
        if (id.empty())
        {
            break;
        }
        int type = jd.Get<int>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":Type"));
        vector<int> rect = jd.GetArray<int>(StringEx::Combine("ImageResults:0:", key, ":", itemIndex, ":Detect:Body:Rect"));
        if (rect.size() >= 4)
        {
            items->insert(pair<string,DetectItem>(id,DetectItem(Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]), type)));
        }
        itemIndex += 1;
    }
}

void DataChannel::HandleDetect(const string& json)
{
    JsonDeserialization jd(json);
    unsigned int channelIndex= jd.Get("ImageResults:0:VideoChannel", static_cast<unsigned int>(_channels.size()));
    if (channelIndex >= 0 && channelIndex < _channels.size())
    {
        long long timeStamp = DateTime::Now().Milliseconds();
        map<string,DetectItem> detectItems;
        DeserializeDetectItems(&detectItems, jd, "Vehicles", timeStamp);
        DeserializeDetectItems(&detectItems, jd, "Bikes", timeStamp);
        DeserializeDetectItems(&detectItems, jd, "Pedestrains", timeStamp);

        string lanesJson;
        for (unsigned int laneIndex=0;laneIndex< _channels[channelIndex].size();++laneIndex)
        {
            IOStatus status = _channels[channelIndex][laneIndex]->Detect(detectItems, timeStamp);
            if (status != IOStatus::UnChanged)
            {
                string laneJson;
                JsonSerialization::Serialize(&laneJson, "laneIndex", laneIndex);
                JsonSerialization::Serialize(&laneJson, "status", (int)status);
                JsonSerialization::Serialize(&laneJson, "type", (int)DetectType::Car);
                JsonSerialization::SerializeItem(&lanesJson, laneJson);
            }
        }

        if (!lanesJson.empty())
        {
            string channelJson;
            JsonSerialization::Serialize(&channelJson, "channelId", channelIndex);
            JsonSerialization::SerializeJsons(&channelJson, "laneStatus", lanesJson);

            string channelsJson;
            JsonSerialization::SerializeItem(&channelsJson, channelJson);

            string ioJson;
            JsonSerialization::Serialize(&ioJson, "timestamp", DateTime::Now().Milliseconds());
            JsonSerialization::SerializeJson(&ioJson, "detail", channelsJson);
         
            _mqtt->Send(IOTopic, ioJson, false);
        }
    }
}

void DataChannel::HandleRecognize(const string& json)
{
    long long timeStamp = DateTime::Now().Milliseconds();
    JsonDeserialization jd(json);
    vector<string> images = jd.GetArray<string>("imgdata_result");

    unsigned int imageIndex=0;
    while (imageIndex<images.size())
    {
        unsigned int channelIndex = jd.Get(StringEx::Combine("l1_result:", imageIndex, ":VideoChannel"), static_cast<unsigned int>(_channels.size()));
        if (channelIndex >= 0 && channelIndex < _channels.size())
        {
            int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Type"));
            if (vehicleType != 0)
            {
                vector<int> vehicleRect = jd.GetArray<int>(StringEx::Combine("l1_result:", imageIndex, ":Detect:Body:Rect"));
                if (vehicleRect.size() >= 4)
                {
                    DetectItem item(Rectangle(Point(vehicleRect[0], vehicleRect[1]), vehicleRect[2], vehicleRect[3]));

                    for (vector<LaneDetector*>::iterator it = _channels[channelIndex].begin(); it != _channels[channelIndex].end(); ++it)
                    {
                        if ((*it)->Contains(item))
                        {
                            VideoVehicle vehicle;
                            vehicle.CarType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Type:TopList:0:Code"));
                            vehicle.CarColor = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Color:TopList:0:Code"));
                            vehicle.CarBrand = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Brand:TopList:0:Name"));
                            vehicle.PlateType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Plate:Type"));
                            vehicle.PlateNumber = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Plate:Licence"));
                            vehicle.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Recognize:Feature:Feature"));
                            vehicle.Image = images[imageIndex];

                            string sendJson;
                            JsonSerialization::Serialize(&sendJson, "dataId", (*it)->DataId());
                            JsonSerialization::Serialize(&sendJson, "timeStamp", timeStamp);
                            JsonSerialization::Serialize(&sendJson, "videoStructType", vehicle.VideoStructType);
                            JsonSerialization::Serialize(&sendJson, "feature", vehicle.Feature);
                            JsonSerialization::Serialize(&sendJson, "image", vehicle.Image);
                            JsonSerialization::Serialize(&sendJson, "carType", vehicle.CarType);
                            JsonSerialization::Serialize(&sendJson, "carColor", vehicle.CarColor);
                            JsonSerialization::Serialize(&sendJson, "carBrand", vehicle.CarBrand);
                            JsonSerialization::Serialize(&sendJson, "plateType", vehicle.PlateType);
                            JsonSerialization::Serialize(&sendJson, "plateNumber", vehicle.PlateNumber);
                            LogPool::Debug("lane:", (*it)->DataId(), "vehicle:", vehicle.CarType);
                            _mqtt->Send(VideoTopic, sendJson, false);
                            break;
                        }
                    }

                    imageIndex += 1;
                    continue;
                }
            }
          
            int bikeType= jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Type"));
            if (bikeType != 0)
            {
                vector<int> bikeRect = jd.GetArray<int>(StringEx::Combine("l1_result:", imageIndex, ":Detect:Body:Rect"));
                if (bikeRect.size() >= 4)
                {
                    DetectItem item(Rectangle(Point(bikeRect[0], bikeRect[1]), bikeRect[2], bikeRect[3]));
                    for (vector<LaneDetector*>::iterator it = _channels[channelIndex].begin(); it != _channels[channelIndex].end(); ++it)
                    {
                        if ((*it)->Contains(item))
                        {
                            VideoBike bike;
                            bike.BikeType = bikeType;
                            bike.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Recognize:Feature:Feature"));
                            bike.Image = images[imageIndex];

                            string sendJson;
                            JsonSerialization::Serialize(&sendJson, "dataId", (*it)->DataId());
                            JsonSerialization::Serialize(&sendJson, "timeStamp", timeStamp);
                            JsonSerialization::Serialize(&sendJson, "videoStructType", bike.VideoStructType);
                            JsonSerialization::Serialize(&sendJson, "feature", bike.Feature);
                            JsonSerialization::Serialize(&sendJson, "image", bike.Image);
                            JsonSerialization::Serialize(&sendJson, "bikeType", bike.BikeType);
                            LogPool::Debug("lane:", (*it)->DataId(), "bike:", bike.BikeType);
                            _mqtt->Send(VideoTopic, sendJson, false);
                        }
                    }

                    imageIndex += 1;
                    continue;
                }
            }
         
            int pedestrainType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Type"));
            if (pedestrainType != 0)
            {
                vector<int> pedestrainRect = jd.GetArray<int>(StringEx::Combine("l1_result:", imageIndex, ":Detect:Body:Rect"));
                if (pedestrainRect.size() >= 4)
                {
                    DetectItem item(Rectangle(Point(pedestrainRect[0], pedestrainRect[1]), pedestrainRect[2], pedestrainRect[3]));
                    for (vector<LaneDetector*>::iterator it = _channels[channelIndex].begin(); it != _channels[channelIndex].end(); ++it)
                    {
                        if ((*it)->Contains(item))
                        {
                            VideoPedestrain pedestrain;
                            pedestrain.Sex = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Sex:TopList:0:Code"));
                            pedestrain.Age = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Age:TopList:0:Code"));
                            pedestrain.UpperColor = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
                            pedestrain.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Feature:Feature"));
                            pedestrain.Image = images[imageIndex];
                            string sendJson;
                            JsonSerialization::Serialize(&sendJson, "dataId", (*it)->DataId());
                            JsonSerialization::Serialize(&sendJson, "timeStamp", timeStamp);
                            JsonSerialization::Serialize(&sendJson, "feature", pedestrain.Feature);
                            JsonSerialization::Serialize(&sendJson, "image", pedestrain.Image);
                            JsonSerialization::Serialize(&sendJson, "videoStructType", pedestrain.VideoStructType);
                            JsonSerialization::Serialize(&sendJson, "sex", pedestrain.Sex);
                            JsonSerialization::Serialize(&sendJson, "age", pedestrain.Age);
                            JsonSerialization::Serialize(&sendJson, "upperColor", pedestrain.UpperColor);
                            LogPool::Debug("lane:", (*it)->DataId(), "pedestrain");
                            _mqtt->Send(VideoTopic, sendJson, false);
                        }
                    }
                    imageIndex += 1;
                    continue;
                }
            }
            break;
        }
        else
        {
            break;
        }
    }
}

void DataChannel::CollectFlow(const DateTime& now)
{
    long long timeStamp = DateTime(now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), 0).Milliseconds();
    lock_guard<mutex> lck(_channelMutex);
    for (unsigned int channelIndex=0;channelIndex<_channels.size();++channelIndex)
    {
        string channelJson;
        JsonSerialization::Serialize(&channelJson, "channelId", channelIndex);
        JsonSerialization::Serialize(&channelJson, "timestamp", timeStamp);
        string lanesJson;
        for (unsigned int laneIndex = 0; laneIndex < _channels[channelIndex].size(); ++laneIndex)
        {
            LaneDetector* detector = _channels[channelIndex][laneIndex];
            LaneItem item = detector->Collect(timeStamp);

            string laneJson;

            JsonSerialization::Serialize(&laneJson, "crossingId", StringEx::ToString(laneIndex+1));

            JsonSerialization::Serialize(&laneJson, "dataId", detector->DataId());
            JsonSerialization::Serialize(&laneJson, "persons", item.Persons);
            JsonSerialization::Serialize(&laneJson, "bikes", item.Bikes);
            JsonSerialization::Serialize(&laneJson, "motorcycles", item.Motorcycles);
            JsonSerialization::Serialize(&laneJson, "cars", item.Cars);
            JsonSerialization::Serialize(&laneJson, "tricycles", item.Tricycles);
            JsonSerialization::Serialize(&laneJson, "buss", item.Buss);
            JsonSerialization::Serialize(&laneJson, "vans", item.Vans);
            JsonSerialization::Serialize(&laneJson, "trucks", item.Trucks);

            JsonSerialization::Serialize(&laneJson, "averageSpeed", static_cast<int>(item.Speed));
            JsonSerialization::Serialize(&laneJson, "headDistance", item.HeadDistance);
            JsonSerialization::Serialize(&laneJson, "headSpace", item.HeadSpace);
            JsonSerialization::Serialize(&laneJson, "timeOccupancy", static_cast<int>(item.TimeOccupancy));
            JsonSerialization::SerializeItem(&lanesJson, laneJson);
        }
        if (!lanesJson.empty())
        {
            JsonSerialization::SerializeJsons(&channelJson, "crossingData", lanesJson);
            _mqtt->Send(FlowTopic, channelJson);
        }
    }
}

void DataChannel::Update(HttpReceivedEventArgs* e)
{
    const string channelUrl("/api/channels");
    if (UrlStartWith(e->Url, channelUrl))
    {
        FlowChannelData data;
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            vector<FlowChannel> channels = data.GetList();
            string channelsJson;
            for (vector<FlowChannel>::iterator cit = channels.begin(); cit != channels.end(); ++cit)
            {
                string channelJson;
                JsonSerialization::Serialize(&channelJson, "channelIndex", cit->ChannelIndex);
                JsonSerialization::Serialize(&channelJson, "channelName", cit->ChannelName);
                JsonSerialization::Serialize(&channelJson, "channelUrl", cit->ChannelUrl);
                JsonSerialization::Serialize(&channelJson, "channelType", cit->ChannelType);
                JsonSerialization::Serialize(&channelJson, "rtspUser", cit->RtspUser);
                JsonSerialization::Serialize(&channelJson, "rtspPassword", cit->RtspPassword);
                JsonSerialization::Serialize(&channelJson, "rtspProtocol", cit->RtspProtocol);
                JsonSerialization::Serialize(&channelJson, "isLoop", cit->IsLoop);

                string lanesJson;
                for (vector<Lane>::const_iterator lit = cit->Lanes.begin(); lit != cit->Lanes.end(); ++lit)
                {
                    string laneJson;
                    JsonSerialization::Serialize(&laneJson, "channelIndex", lit->ChannelIndex);
                    JsonSerialization::Serialize(&laneJson, "laneId", lit->LaneId);
                    JsonSerialization::Serialize(&laneJson, "laneName", lit->LaneName);
                    JsonSerialization::Serialize(&laneJson, "laneIndex", lit->LaneIndex);
                    JsonSerialization::Serialize(&laneJson, "laneType", lit->LaneType);
                    JsonSerialization::Serialize(&laneJson, "direction", lit->Direction);
                    JsonSerialization::Serialize(&laneJson, "flowDirection", lit->FlowDirection);
                    JsonSerialization::Serialize(&laneJson, "length", lit->Length);
                    JsonSerialization::Serialize(&laneJson, "iOIp", lit->IOIp);
                    JsonSerialization::Serialize(&laneJson, "iOPort", lit->IOPort);
                    JsonSerialization::Serialize(&laneJson, "iOIndex", lit->IOIndex);
                    JsonSerialization::Serialize(&laneJson, "detectLine", lit->DetectLine);
                    JsonSerialization::Serialize(&laneJson, "stopLine", lit->StopLine);
                    JsonSerialization::Serialize(&laneJson, "laneLine1", lit->LaneLine1);
                    JsonSerialization::Serialize(&laneJson, "laneLine2", lit->LaneLine2);
                    JsonSerialization::SerializeItem(&lanesJson, laneJson);
                }
                JsonSerialization::SerializeJsons(&channelJson, "lanes", lanesJson);

                JsonSerialization::SerializeItem(&channelsJson, channelJson);
            }
            e->Code = HttpCode::OK;
            e->ResponseJson = channelsJson;
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            JsonDeserialization jd(e->RequestJson);
            int channelIndex = 0;
            vector<FlowChannel> channels;
            while (true)
            {
                FlowChannel channel;
                channel.ChannelIndex = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":channelIndex"));
                if (channel.ChannelIndex==0)
                {
                    break;
                }
                channel.ChannelName = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":channelName"));
                channel.ChannelUrl = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":channelUrl"));
                channel.ChannelType = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":channelType"));
                channel.RtspUser = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":rtspUser"));
                channel.RtspPassword = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":rtspPassword"));
                channel.RtspProtocol = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":rtspProtocol"));
                channel.IsLoop = jd.Get<bool>(StringEx::Combine("channels:", channelIndex, ":isLoop"));

                int laneIndex = 0;
                while (true)
                {
                    Lane lane;
                    lane.LaneId = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneId"));
                    if (lane.LaneId.empty())
                    {
                        break;
                    }
                    lane.ChannelIndex = channel.ChannelIndex;
                    lane.LaneName = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneName"));
                    lane.LaneIndex = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneIndex"));
                    lane.LaneType = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneType"));
                    lane.Direction = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":direction"));
                    lane.FlowDirection = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":flowDirection"));
                    lane.Length = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":length"));
                    lane.IOIp = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":iOIp"));
                    lane.IOPort = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":iOPort"));
                    lane.IOIndex = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":iOIndex"));
                    lane.DetectLine = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":detectLine"));
                    lane.StopLine = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":stopLine"));
                    lane.LaneLine1 = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneLine1"));
                    lane.LaneLine2 = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneLine2"));
                    channel.Lanes.push_back(lane);
                    laneIndex += 1;
                }
                channels.push_back(channel);
                channelIndex += 1;
            }
            FlowChannelData data;
            data.SetList(channels);
            for (vector<FlowChannel>::iterator it = channels.begin(); it != channels.end(); ++it)
            {
                UpdateChannel(*it);
            }
            e->Code = HttpCode::OK;
        }
    }
}

bool DataChannel::UrlStartWith(const string& url, const string& key)
{
    return url.size() >= key.size() && url.substr(0, key.size()).compare(key) == 0;
}



