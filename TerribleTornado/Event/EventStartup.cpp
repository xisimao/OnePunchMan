#include "EventStartup.h"

using namespace std;
using namespace OnePunchMan;

EventStartup::EventStartup()
    :TrafficStartup()
{
    TrafficData::Init("event.db");
}

EventStartup::~EventStartup()
{
    for (unsigned int i = 0; i < _detectors.size(); ++i)
    {
        delete _detectors[i];
    }
}

void EventStartup::UpdateDb()
{
    EventChannelData data;
    data.UpdateDb();
}

void EventStartup::InitThreads(MqttChannel* mqtt, vector<DecodeChannel*>* decodes, vector<TrafficDetector*>* detectors, vector<DetectChannel*>* detects, vector<RecognChannel*>* recogns)
{
    for (int i = 0; i < ChannelCount; ++i)
    {
        EventDetector* detector = new EventDetector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, mqtt);
        _detectors.push_back(detector);
        detectors->push_back(detector);
        DecodeChannel* decode = new DecodeChannel(i + 1);
        decodes->push_back(decode);
    }

    for (int i = 0; i < DetectCount; ++i)
    {
        DetectChannel* detect = new DetectChannel(i, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight);
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
}

void EventStartup::InitChannels()
{
    EventChannelData data;
    for (int i = 0; i < ChannelCount; ++i)
    {
        EventChannel channel = data.Get(i + 1);
        if (!channel.ChannelUrl.empty())
        {
            _decodes[i]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), channel.Loop);
            _detectors[i]->UpdateChannel(channel);
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
        JsonSerialization::SerializeValue(&channelJson, "channelIndex", channel.ChannelIndex);
        JsonSerialization::SerializeValue(&channelJson, "channelName", channel.ChannelName);
        JsonSerialization::SerializeValue(&channelJson, "channelUrl", channel.ChannelUrl);
        JsonSerialization::SerializeValue(&channelJson, "rtmpUrl", channel.RtmpUrl(host));
        JsonSerialization::SerializeValue(&channelJson, "channelType", channel.ChannelType);
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

string EventStartup::CheckChannel(EventChannel* channel)
{
    if (ChannelIndexEnable(channel->ChannelIndex))
    {
        return string();
    }
    else
    {
        return GetErrorJson("channelIndex", StringEx::Combine("channelIndex is limited to 1-", ChannelCount));
    }
}

void EventStartup::SetDevice(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    int channelIndex = 0;
    vector<EventChannel> channels;
    map<int, EventChannel> tempChannels;
    while (true)
    {
        EventChannel channel;
        channel.ChannelIndex = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":channelIndex"));
        if (channel.ChannelIndex == 0)
        {
            break;
        }
        channel.ChannelName = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":channelName"));
        channel.ChannelUrl = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":channelUrl"));
        channel.ChannelType = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":channelType"));
        //设置设备接口无法进行调试
        channel.Loop = true;
        channel.OutputImage = false;
        channel.OutputReport = false;
        int laneIndex = 0;
        while (true)
        {
            EventLane lane;
            lane.LaneIndex = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneIndex"));
            if (lane.LaneIndex==0)
            {
                break;
            }
            lane.ChannelIndex = channel.ChannelIndex;
            lane.LaneName = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneName"));
            lane.LaneType = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneType"));
            lane.Region = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":region"));
            lane.Line = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":line"));
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
        channelIndex += 1;
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
                _decodes[i]->UpdateChannel(it->second.ChannelUrl, it->second.RtmpUrl("127.0.0.1"), it->second.Loop);
                _detectors[i]->UpdateChannel(it->second);
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
    channel.ChannelIndex = jd.Get<int>("channelIndex");
    channel.ChannelName = jd.Get<string>("channelName");
    channel.ChannelUrl = jd.Get<string>("channelUrl");
    channel.ChannelType = jd.Get<int>("channelType");
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
        _decodes[channel.ChannelIndex - 1]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"),channel.Loop);
        _detectors[channel.ChannelIndex - 1]->UpdateChannel(channel);
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