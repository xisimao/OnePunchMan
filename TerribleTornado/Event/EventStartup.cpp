#include "EventStartup.h"

using namespace std;
using namespace OnePunchMan;

EventStartup::EventStartup()
    :TrafficStartup(), _encode(ChannelCount), _data(NULL)
{

}

EventStartup::~EventStartup()
{
    for (unsigned int i = 0; i < _detectors.size(); ++i)
    {
        delete _detectors[i];
    }
}

void EventStartup::Update(HttpReceivedEventArgs* e)
{
    if (UrlStartWith(e->Url, "/api/data"))
    {
        int channelIndex = StringEx::Convert<int>(GetId(e->Url, "/api/data"));
        vector<EventData> datas=EventDataChannel::GetDatas(channelIndex);
        for (vector<EventData>::iterator it = datas.begin(); it != datas.end(); ++it)
        {
            string json;
            JsonSerialization::SerializeValue(&json, "id", it->Id);
            string guid = it->Guid;
            JsonSerialization::SerializeValue(&json, "channelIndex", it->ChannelIndex);
            JsonSerialization::SerializeValue(&json, "laneIndex", it->LaneIndex);
            JsonSerialization::SerializeValue(&json, "timeStamp", it->TimeStamp);
            JsonSerialization::SerializeValue(&json, "type", it->Type);
            JsonSerialization::SerializeValue(&json, "image1", EventData::GetImageLink(guid, 1));
            JsonSerialization::SerializeValue(&json, "image2", EventData::GetImageLink(guid, 2));
            JsonSerialization::SerializeValue(&json, "video", EventData::GetVideoLink(guid));
            JsonSerialization::AddClassItem(&e->ResponseJson, json);
        }
        e->Code = HttpCode::OK;
    }
    else
    {
        TrafficStartup::Update(e);
    }
}

void EventStartup::UpdateDb()
{
    EventChannelData data;
    data.UpdateDb();
}

void EventStartup::InitThreads(MqttChannel* mqtt, vector<DecodeChannel*>* decodes, vector<TrafficDetector*>* detectors, vector<DetectChannel*>* detects, vector<RecognChannel*>* recogns,int loginHandler)
{
    _data = new EventDataChannel(mqtt);
    for (int i = 0; i < ChannelCount; ++i)
    {
        EventDetector* detector = new EventDetector(DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, mqtt, &_encode,_data);
        _detectors.push_back(detector);
        detectors->push_back(detector);
        DecodeChannel* decode = new DecodeChannel(i + 1, loginHandler, &_encode);
        decodes->push_back(decode);
    }

    for (int i = 0; i < DetectCount; ++i)
    {
        DetectChannel* detect = new DetectChannel(i, DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight);
        detects->push_back(detect);
    }

    for (int i = 0; i < DetectCount; ++i)
    {
        detects->at(i)->SetRecogn(NULL);
        for (int j = 0; j < ChannelCount / DetectCount; ++j)
        {
            int channelIndex = i + (j * DetectCount) + 1;
            detects->at(i)->AddChannel(channelIndex, decodes->at(channelIndex - 1), detectors->at(channelIndex - 1));
        }
    }
    _encode.Start();
    _data->Start();
}

void EventStartup::InitChannels()
{
    EventChannelData data;
    for (int i = 0; i < ChannelCount; ++i)
    {
        EventChannel channel = data.Get(i + 1);
        if (!channel.ChannelUrl.empty())
        {
            unsigned char taskId=_decodes[i]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"),static_cast<ChannelType>(channel.ChannelType), channel.Loop);
            _detectors[i]->UpdateChannel(taskId,channel);
        }
    }
}

string EventStartup::GetChannelJson(const string& host,int channelIndex)
{
    string channelJson;
    EventChannelData data;
    EventChannel channel=data.Get(channelIndex);
    if (!channel.ChannelUrl.empty())
    {
        FillChannelJson(&channelJson, &channel, host);
        if (ChannelIndexEnable(channel.ChannelIndex))
        {
            JsonSerialization::SerializeValue(&channelJson, "lanesInited", _detectors[channel.ChannelIndex - 1]->LanesInited());
        }

        string lanesJson;
        for (vector<EventLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
        {
            string laneJson;
            JsonSerialization::SerializeValue(&laneJson, "channelIndex", lit->ChannelIndex);
            JsonSerialization::SerializeValue(&laneJson, "laneIndex", lit->LaneIndex);
            JsonSerialization::SerializeValue(&laneJson, "laneName", lit->LaneName);
            JsonSerialization::SerializeValue(&laneJson, "laneType", lit->LaneType);
            JsonSerialization::SerializeValue(&laneJson, "region", lit->Region);
            JsonSerialization::SerializeValue(&laneJson, "line", lit->Line);
            JsonSerialization::AddClassItem(&lanesJson, laneJson);
        }
        JsonSerialization::SerializeArray(&channelJson, "lanes", lanesJson);
    }
    return channelJson;
}

void EventStartup::SetDevice(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    int itemIndex = 0;
    vector<EventChannel> channels;
    map<int, EventChannel> tempChannels;
    while (true)
    {
        EventChannel channel;
        FillChannel(&channel, jd, itemIndex);
        if (channel.ChannelIndex == 0)
        {
            break;
        }
        int laneIndex = 0;
        while (true)
        {
            EventLane lane;
            lane.LaneIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":laneIndex"));
            if (lane.LaneIndex==0)
            {
                break;
            }
            lane.ChannelIndex = channel.ChannelIndex;
            lane.LaneName = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":laneName"));
            lane.LaneType = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":laneType"));
            lane.Region = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":region"));
            lane.Line = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":line"));
            channel.Lanes.push_back(lane);
            laneIndex += 1;
        }
        string error = CheckChannel(&channel);
        if (!error.empty())
        {
            e->Code = HttpCode::BadRequest;
            e->ResponseJson = error;
            return;
        }
        channels.push_back(channel);
        tempChannels.insert(pair<int, EventChannel>(channel.ChannelIndex, channel));
        itemIndex += 1;
    }
    EventChannelData data;
    if (data.SetList(channels))
    {
        for (int i = 0; i < ChannelCount; ++i)
        {
            map<int, EventChannel>::iterator it = tempChannels.find(i + 1);
            if (it == tempChannels.end())
            {
                _decodes[i]->ClearChannel();
                _detectors[i]->ClearChannel();
            }
            else
            {
                unsigned char taskId=_decodes[i]->UpdateChannel(it->second.ChannelUrl, it->second.RtmpUrl("127.0.0.1"), static_cast<ChannelType>(it->second.ChannelType), it->second.Loop);
                _detectors[i]->UpdateChannel(taskId,it->second);
            }
        }
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = GetErrorJson("db", data.LastError());
    }
}

void EventStartup::SetChannel(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);

    EventChannel channel;
    FillChannel(&channel, jd);
    int laneIndex = 0;
    while (true)
    {
        EventLane lane;
        lane.LaneIndex = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":laneIndex"));
        if (lane.LaneIndex==0)
        {
            break;
        }
        lane.ChannelIndex = channel.ChannelIndex;
        lane.LaneName = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":laneName"));
        lane.LaneType = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":laneType"));
        lane.Region = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":region"));
        lane.Line = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":line"));
        channel.Lanes.push_back(lane);
        laneIndex += 1;
    }
    string error = CheckChannel(&channel);
    if (!error.empty())
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = error;
        return;
    }
    EventChannelData data;
    if (data.Set(channel))
    {
        unsigned char taskId=_decodes[channel.ChannelIndex - 1]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), static_cast<ChannelType>(channel.ChannelType),channel.Loop);
        _detectors[channel.ChannelIndex - 1]->UpdateChannel(taskId,channel);
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = GetErrorJson("db", data.LastError());
    }
}

bool EventStartup::DeleteChannel(int channelIndex)
{
    EventChannelData data;
    if (data.Delete(channelIndex))
    {
        _decodes[channelIndex - 1]->ClearChannel();
        _detectors[channelIndex - 1]->ClearChannel();
        return true;
    }
    else
    {
        return false;
    }
}