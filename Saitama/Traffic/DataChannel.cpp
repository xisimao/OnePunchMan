#include "DataChannel.h"

using namespace std;
using namespace Saitama;

const string DataChannel::DetectTopic("aiservice/level_1_imageresult");
const string DataChannel::ChannelTopic("manager2analysis_crossingparam");
const string DataChannel::ChannelRequestTopic("analysis2manager_crossingparam_request");
const string DataChannel::TrafficTopic("analysis2manager_crossingflows");
const string DataChannel::IOTopic("analysis2manager_deviceIO");

DataChannel::DataChannel(const std::string& ip, int port)
    :ThreadObject("data"), _mqtt(NULL),_ip(ip),_port(port)
{
    _lastMinute = DateTime::Now().Minute();
}

void DataChannel::Update(MqttReceivedEventArgs* e)
{
    if (e->Topic[0] == 'a')
    {
        HandleDetect(e->Message);
    }
    else
    {
        HandleChannel(e->Message);
    }
}

void DataChannel::StartCore()
{   
    //初始化通道
    for (int i = 0; i < 8; ++i)
    {
        Channel* channel = new Channel();
        _channels.push_back(channel);
    }

    //初始化mqtt
    vector<string> topics;
    topics.push_back(DetectTopic);
    topics.push_back(ChannelTopic);
    _mqtt = new MqttChannel(_ip, _port, topics);
    _mqtt->MqttReceived.Subscribe(this);
    _mqtt->Start();

    bool init = false;
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

        //初始化通道集合
        if (!init&& _mqtt->Send(ChannelRequestTopic, "1"))
        {
            init = true;
        }
    }

    _mqtt->Stop();
    delete _mqtt;
    for (vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        delete (*it);
    }
}

vector<DetectItem> DataChannel::GetDetectItems(const std::string& json, const std::string& key, long long timeStamp)
{
    vector<string> values;
    JsonFormatter::Deserialize(json, key, &values);
    vector<DetectItem> items;
    for (vector<string>::iterator it = values.begin(); it != values.end(); ++it)
    {
        string id;
        JsonFormatter::Deserialize(*it, "GUID", &id);
        int type;
        JsonFormatter::Deserialize(*it, "Type", &type);
        string detect;
        JsonFormatter::Deserialize(*it, "Detect", &detect);
        string body;
        JsonFormatter::Deserialize(detect, "Body", &body);
        vector<int> datas;
        JsonFormatter::Deserialize(body, "Rect", &datas);
        items.push_back(DetectItem(id, timeStamp, type, Rectangle(datas[0], datas[1], datas[2], datas[3])));
    }
    return items;

}

void DataChannel::HandleDetect(const string& json)
{
    int channelIndex = -1;
    JsonFormatter::Deserialize(json, "VideoChannel", &channelIndex);
    if (channelIndex >= 0 && channelIndex < _channels.size())
    {
        long long timeStamp = DateTime::Now().Milliseconds();
        vector<DetectItem> vehicles = GetDetectItems(json, "Vehicles", timeStamp);
        vector<DetectItem> bikes = GetDetectItems(json, "Bikes", timeStamp);
        vector<DetectItem> pedestrains = GetDetectItems(json, "Pedestrains", timeStamp);

        vector<IOItem> items = _channels[channelIndex]->Detect(vehicles, bikes, pedestrains);

        for (vector<IOItem>::iterator it = items.begin(); it != items.end(); ++it)
        {
            string laneStatusJson;
            JsonFormatter::Serialize(&laneStatusJson, "laneId", it->LaneId);
            JsonFormatter::Serialize(&laneStatusJson, "laneIndex", it->LaneIndex);
            JsonFormatter::Serialize(&laneStatusJson, "status", it->Status ? 1 : 0);
            JsonFormatter::Serialize(&laneStatusJson, "type", (int)DetectType::Car);
            std::vector<string> laneStatusJsons;
            laneStatusJsons.push_back(laneStatusJson);

            string channelJson;
            JsonFormatter::Serialize(&channelJson, "channelId", it->ChannelIndex);
            JsonFormatter::Serialize(&channelJson, "channelReadlId", it->ChannelId);
            JsonFormatter::SerializeArray(&channelJson, "laneStatus", laneStatusJsons);

            std::vector<string> channelJsons;
            channelJsons.push_back(channelJson);

            string json;
            JsonFormatter::Serialize(&json, "timestamp", DateTime::Now().Milliseconds());
            JsonFormatter::SerializeArray(&json, "detail", channelJsons);
            LogPool::Debug("channel:", it->ChannelIndex, "lane:", it->LaneIndex, "io:", it->Status);
            _mqtt->Send(IOTopic, json, false);
        }
    }
}

void DataChannel::HandleChannel(const string& json)
{
    int channelIndex = -1;
    JsonFormatter::Deserialize(json, "ChannelIndex", &channelIndex);
    if (channelIndex >= 0 && channelIndex < _channels.size())
    {
        LogPool::Debug("init channel ", channelIndex);
        string channelId;
        JsonFormatter::Deserialize(json, "ChannelID", &channelId);
        int channelIndex = -1;
        JsonFormatter::Deserialize(json, "ChannelIndex", &channelIndex);
        vector<string> lanesValue;
        JsonFormatter::Deserialize(json, "CrossingFlows", &lanesValue);
        vector<Lane*> lanes;
        for (vector<string>::iterator it = lanesValue.begin(); it != lanesValue.end(); ++it)
        {
            vector<string> pointPairs;
            JsonFormatter::Deserialize(*it, "region", &pointPairs);
            string laneId;
            JsonFormatter::Deserialize(*it, "laneId", &laneId);
            int laneIndex = -1;
            JsonFormatter::Deserialize(*it, "laneIndex", &laneIndex);
            vector<Point> points;
            for (vector<string>::iterator pit = pointPairs.begin(); pit != pointPairs.end(); ++pit)
            {
                vector<int> values;
                JsonFormatter::DeserializeValue(*pit, &values);
                if (values.size() == 2)
                {
                    points.push_back(Point(values[0], values[1]));
                }
            }
            lanes.push_back(new Lane(laneId, laneIndex, Polygon(points)));
        }
        _channels[channelIndex]->Id = channelId;
        _channels[channelIndex]->Index = channelIndex;
        _channels[channelIndex]->UpdateLanes(lanes);
    }
}

void DataChannel::CollectFlow(const DateTime& now)
{
    long long timeStamp = DateTime(now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), 0).Milliseconds();
    for (vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        vector<LaneItem> lanes = (*it)->Collect();
        if (!lanes.empty())
        {
            vector<string> values;
            string channelJson;
            JsonFormatter::Serialize(&channelJson, "channelId", (*it)->Index);
            JsonFormatter::Serialize(&channelJson, "channelRealId", (*it)->Id);
            JsonFormatter::Serialize(&channelJson, "timestamp", timeStamp);

            for (vector<LaneItem>::iterator lit = lanes.begin(); lit != lanes.end(); ++lit)
            {
                string laneJson;

                JsonFormatter::Serialize(&laneJson, "crossingId", StringEx::ToString(lit->Index));
                JsonFormatter::Serialize(&laneJson, "laneId", lit->Id);

                JsonFormatter::Serialize(&laneJson, "persons", lit->Persons);
                JsonFormatter::Serialize(&laneJson, "bikes", lit->Bikes);
                JsonFormatter::Serialize(&laneJson, "motorcycles", lit->Motorcycles);
                JsonFormatter::Serialize(&laneJson, "cars", lit->Cars);
                JsonFormatter::Serialize(&laneJson, "tricycles", lit->Tricycles);
                JsonFormatter::Serialize(&laneJson, "buss", lit->Buss);
                JsonFormatter::Serialize(&laneJson, "vans", lit->Vans);
                JsonFormatter::Serialize(&laneJson, "trucks", lit->Trucks);

                JsonFormatter::Serialize(&laneJson, "averageSpeed", lit->Speed);
                JsonFormatter::Serialize(&laneJson, "headDistance", lit->HeadDistance);
                JsonFormatter::Serialize(&laneJson, "timeOccupancy", lit->TimeOccupancy);
                LogPool::Debug("channel:", (*it)->Index, "lane:", lit->Index, "cars:", lit->Cars + lit->Tricycles + lit->Buss + lit->Vans + lit->Trucks, "bikes:", lit->Bikes + lit->Motorcycles, "persons:", lit->Persons, lit->Speed, "km/h ", lit->HeadDistance, "sec ", lit->TimeOccupancy, "%");

                values.push_back(laneJson);
            }
            JsonFormatter::SerializeArray(&channelJson, "crossingData", values);

            _mqtt->Send(TrafficTopic, channelJson);
        }
    }
}