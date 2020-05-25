#include "EventStartup.h"

using namespace std;
using namespace OnePunchMan;

EventStartup::~EventStartup()
{
    for (unsigned int i = 0; i < _detectors.size(); ++i)
    {
        delete _detectors[i];
    }
}

void EventStartup::InitDetectors(MqttChannel* mqtt, vector<DetectChannel*>* detects, vector<RecognChannel*>* recogns)
{
    vector<TrafficDetector*> detectors;
    for (int i = 0; i < ChannelCount; ++i)
    {
        EventDetector* detector = new EventDetector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight,mqtt,false);
        _detectors.push_back(detector);
        detectors.push_back(detector);
    }
    for (int i = 0; i < ChannelCount; ++i)
    {
        DetectChannel* detect = new DetectChannel(i + 1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight,NULL, detectors[i]);
        detects->push_back(detect);
    }
}

void EventStartup::InitDecodes()
{
    for (int i = 0; i < ChannelCount; ++i)
    {
        EventChannelData data;
        EventChannel channel = data.Get(i + 1);
        if (!channel.ChannelUrl.empty())
        {
            SetDecode(channel.ChannelIndex, channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"));
            _detectors[i]->UpdateChannel(channel);
        }
        if (_softwareVersion.empty())
        {
            _softwareVersion = data.Version();
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
        JsonSerialization::Serialize(&channelJson, "channelIndex", channel.ChannelIndex);
        JsonSerialization::Serialize(&channelJson, "channelName", channel.ChannelName);
        JsonSerialization::Serialize(&channelJson, "channelUrl", channel.ChannelUrl);
        JsonSerialization::Serialize(&channelJson, "rtmpUrl", channel.RtmpUrl(host));
        JsonSerialization::Serialize(&channelJson, "channelType", channel.ChannelType);
        if (ChannelIndexEnable(channel.ChannelIndex))
        {
            JsonSerialization::Serialize(&channelJson, "lanesInited", _detectors[channel.ChannelIndex - 1]->LanesInited());
        }

        string lanesJson;
        for (vector<EventLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
        {
            string laneJson;
            JsonSerialization::Serialize(&laneJson, "channelIndex", lit->ChannelIndex);
            JsonSerialization::Serialize(&laneJson, "laneIndex", lit->LaneIndex);
            JsonSerialization::Serialize(&laneJson, "laneName", lit->LaneName);
            JsonSerialization::Serialize(&laneJson, "laneType", lit->LaneType);
            JsonSerialization::Serialize(&laneJson, "region", lit->Region);
            JsonSerialization::Serialize(&laneJson, "line", lit->Line);
            JsonSerialization::SerializeItem(&lanesJson, laneJson);
        }
        JsonSerialization::SerializeJsons(&channelJson, "lanes", lanesJson);
    }
    return channelJson;

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
        string error = CheckChannel(channel.ChannelIndex);
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
                DeleteDecode(i + 1);
                _detectors[i]->ClearChannel();
            }
            else
            {
                SetDecode(i + 1, it->second.ChannelUrl, it->second.RtmpUrl("127.0.0.1"));
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
    string error = CheckChannel(channel.ChannelIndex);
    if (!error.empty())
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = error;
        return;
    }
    EventChannelData data;
    if (data.Set(channel))
    {
        SetDecode(channel.ChannelIndex, channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"));
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
        DeleteDecode(channelIndex);
        _detectors[channelIndex - 1]->ClearChannel();
        return true;
    }
    else
    {
        return false;
    }
}