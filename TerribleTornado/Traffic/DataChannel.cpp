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
        VideoDetector* channel = new VideoDetector();
        channel->Index = i;
        _channels.push_back(channel);
    }
    FlowChannelData data;
    vector<FlowChannel> channels = data.GetList();
    for (vector<FlowChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if (it->ChannelIndex >= 1 && static_cast<unsigned int>(it->ChannelIndex) <= _channels.size())
        {
            _channels[it->ChannelIndex - 1]->Url = it->ChannelUrl;
            _channels[it->ChannelIndex - 1]->UpdateLanes(it->Lanes);
        }
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
    for (vector<VideoDetector*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        delete (*it);
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
    int channelIndex= jd.Get("ImageResults:0:VideoChannel", -1);
    if (channelIndex >= 0 && static_cast<unsigned int>(channelIndex) < _channels.size())
    {
        long long timeStamp = DateTime::Now().Milliseconds();
        map<string,DetectItem> detectItems;
        DeserializeDetectItems(&detectItems, jd, "Vehicles", timeStamp);
        DeserializeDetectItems(&detectItems, jd, "Bikes", timeStamp);
        DeserializeDetectItems(&detectItems, jd, "Pedestrains", timeStamp);

        vector<IOItem> iOItems = _channels[channelIndex]->Detect(detectItems,timeStamp);

        for (vector<IOItem>::iterator it = iOItems.begin(); it != iOItems.end(); ++it)
        {
       
            string laneJson;
            JsonSerialization::Serialize(&laneJson, "laneId", it->LaneId);
            JsonSerialization::Serialize(&laneJson, "laneIndex", it->LaneIndex);
            JsonSerialization::Serialize(&laneJson, "status", it->Status ? 1 : 0);
            JsonSerialization::Serialize(&laneJson, "type", (int)DetectType::Car);
            string lanesJson;
            JsonSerialization::SerializeItem(&lanesJson, laneJson);

            string channelJson;
            JsonSerialization::Serialize(&channelJson, "channelId", it->ChannelIndex);
            JsonSerialization::Serialize(&channelJson, "channelReadlId", it->ChannelId);
            JsonSerialization::SerializeJsons(&channelJson, "laneStatus", lanesJson);

            string channelsJson;
            JsonSerialization::SerializeItem(&channelsJson, channelJson);

            string ioJson;
            JsonSerialization::Serialize(&ioJson, "timestamp", DateTime::Now().Milliseconds());
            JsonSerialization::SerializeJson(&ioJson, "detail", channelsJson);
            //LogPool::Debug("channel:", it->ChannelIndex, "lane:", it->LaneIndex, "io:", it->Status);
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
        int channelIndex = jd.Get(StringEx::Combine("l1_result:", imageIndex, ":VideoChannel"), -1);
        if (channelIndex >= 0 && static_cast<unsigned int>(channelIndex) < _channels.size())
        {
            int vehicleType = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Vehicles:0:Type"));
            if (vehicleType != 0)
            {
                vector<int> vehicleRect = jd.GetArray<int>(StringEx::Combine("l1_result:", imageIndex, ":Detect:Body:Rect"));
                if (vehicleRect.size() >= 4)
                {
                    DetectItem item(Rectangle(Point(vehicleRect[0], vehicleRect[1]), vehicleRect[2], vehicleRect[3]));
                    string laneId = _channels[channelIndex]->Contains(item);
                    if (!laneId.empty())
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
                        JsonSerialization::Serialize(&sendJson, "ChannelId", _channels[channelIndex]->Url);
                        JsonSerialization::Serialize(&sendJson, "LaneId", laneId);
                        JsonSerialization::Serialize(&sendJson, "TimeStamp", timeStamp);
                        JsonSerialization::Serialize(&sendJson, "VideoStructType", vehicle.VideoStructType);
                        JsonSerialization::Serialize(&sendJson, "Feature", vehicle.Feature);
                        JsonSerialization::Serialize(&sendJson, "Image", vehicle.Image);
                        JsonSerialization::Serialize(&sendJson, "CarType", vehicle.CarType);
                        JsonSerialization::Serialize(&sendJson, "CarColor", vehicle.CarColor);
                        JsonSerialization::Serialize(&sendJson, "CarBrand", vehicle.CarBrand);
                        JsonSerialization::Serialize(&sendJson, "PlateType", vehicle.PlateType);
                        JsonSerialization::Serialize(&sendJson, "PlateNumber", vehicle.PlateNumber);
                        _mqtt->Send(VideoTopic, sendJson, false);
                    };
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
                    string laneId = _channels[channelIndex]->Contains(item);
                    if (!laneId.empty())
                    {
                        VideoBike bike;
                        bike.BikeType = bikeType;
                        bike.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Bikes:0:Recognize:Feature:Feature"));
                        bike.Image = images[imageIndex];

                        string sendJson;
                        JsonSerialization::Serialize(&sendJson, "ChannelId", _channels[channelIndex]->Url);
                        JsonSerialization::Serialize(&sendJson, "LaneId", laneId);
                        JsonSerialization::Serialize(&sendJson, "TimeStamp", timeStamp);
                        JsonSerialization::Serialize(&sendJson, "VideoStructType", bike.VideoStructType);
                        JsonSerialization::Serialize(&sendJson, "Feature", bike.Feature);
                        JsonSerialization::Serialize(&sendJson, "Image", bike.Image);
                        JsonSerialization::Serialize(&sendJson, "BikeType", bike.BikeType);
                        _mqtt->Send(VideoTopic, sendJson, false);
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
                    string laneId = _channels[channelIndex]->Contains(item);
                    if (!laneId.empty())
                    {
                        VideoPedestrain pedestrain;
                        pedestrain.Sex = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Sex:TopList:0:Code"));
                        pedestrain.Age = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Age:TopList:0:Code"));
                        pedestrain.UpperColor = jd.Get<int>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:UpperColor:TopList:0:Code"));
                        pedestrain.Feature = jd.Get<string>(StringEx::Combine("ImageResults:", imageIndex, ":Pedestrains:0:Recognize:Feature:Feature"));
                        pedestrain.Image = images[imageIndex];
                        LogPool::Information("people", pedestrain.Sex, pedestrain.Age, pedestrain.UpperColor);
                        string sendJson;
                        JsonSerialization::Serialize(&sendJson, "ChannelId", _channels[channelIndex]->Url);
                        JsonSerialization::Serialize(&sendJson, "LaneId", laneId);
                        JsonSerialization::Serialize(&sendJson, "TimeStamp", timeStamp);
                        JsonSerialization::Serialize(&sendJson, "Feature", pedestrain.Feature);
                        JsonSerialization::Serialize(&sendJson, "Image", pedestrain.Image);
                        JsonSerialization::Serialize(&sendJson, "VideoStructType", pedestrain.VideoStructType);
                        JsonSerialization::Serialize(&sendJson, "Sex", pedestrain.Sex);
                        JsonSerialization::Serialize(&sendJson, "Age", pedestrain.Age);
                        JsonSerialization::Serialize(&sendJson, "UpperColor", pedestrain.UpperColor);
                        _mqtt->Send(VideoTopic, sendJson, false);
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
    for (vector<VideoDetector*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        vector<LaneItem> lanes = (*it)->Collect();
        if (!lanes.empty())
        {
            string channelJson;
            JsonSerialization::Serialize(&channelJson, "channelId", (*it)->Index);
            JsonSerialization::Serialize(&channelJson, "channelRealId", (*it)->Url);
            JsonSerialization::Serialize(&channelJson, "timestamp", timeStamp);

            string lanesJson;
            for (vector<LaneItem>::iterator lit = lanes.begin(); lit != lanes.end(); ++lit)
            {
                LogPool::Debug("channel:", (*it)->Index, "lane:", lit->Index, "vehicles:", lit->Cars + lit->Tricycles + lit->Buss + lit->Vans + lit->Trucks, "bikes:", lit->Bikes + lit->Motorcycles, "persons:", lit->Persons, lit->Speed, "km/h ", lit->HeadDistance, "sec ", lit->TimeOccupancy, "%");

                string laneJson;

                JsonSerialization::Serialize(&laneJson, "crossingId", StringEx::ToString(lit->Index));
                JsonSerialization::Serialize(&laneJson, "laneId", lit->Id);

                JsonSerialization::Serialize(&laneJson, "persons", lit->Persons);
                JsonSerialization::Serialize(&laneJson, "bikes", lit->Bikes);
                JsonSerialization::Serialize(&laneJson, "motorcycles", lit->Motorcycles);
                JsonSerialization::Serialize(&laneJson, "cars", lit->Cars);
                JsonSerialization::Serialize(&laneJson, "tricycles", lit->Tricycles);
                JsonSerialization::Serialize(&laneJson, "buss", lit->Buss);
                JsonSerialization::Serialize(&laneJson, "vans", lit->Vans);
                JsonSerialization::Serialize(&laneJson, "trucks", lit->Trucks);

                JsonSerialization::Serialize(&laneJson, "averageSpeed", static_cast<int>(lit->Speed));
                JsonSerialization::Serialize(&laneJson, "headDistance", lit->HeadDistance);
                JsonSerialization::Serialize(&laneJson, "headSpace", lit->HeadSpace);
                JsonSerialization::Serialize(&laneJson, "timeOccupancy", static_cast<int>(lit->TimeOccupancy));
                JsonSerialization::SerializeItem(&lanesJson, laneJson);
            }
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
                if (it->ChannelIndex >= 1 && static_cast<unsigned int>(it->ChannelIndex)<= _channels.size())
                {
                    _channels[it->ChannelIndex - 1]->Url = it->ChannelUrl;
                    _channels[it->ChannelIndex - 1]->UpdateLanes(it->Lanes);
                }
            }
            e->Code = HttpCode::OK;
        }
    }
}

bool DataChannel::UrlStartWith(const string& url, const string& key)
{
    return url.size() >= key.size() && url.substr(0, key.size()).compare(key) == 0;
}



