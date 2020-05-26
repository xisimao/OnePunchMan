#include "FlowStartup.h"

using namespace std;
using namespace OnePunchMan;

FlowStartup::~FlowStartup()
{
    for (unsigned int i = 0; i < _detectors.size(); ++i)
    {
        delete _detectors[i];
    }
}

void FlowStartup::InitDetectors(MqttChannel* mqtt, vector<DetectChannel*>* detects, vector<RecognChannel*>* recogns)
{
    vector<TrafficDetector*> detectors;
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowDetector* detector = new FlowDetector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, mqtt, false);
        _detectors.push_back(detector);
        detectors.push_back(detector);
    }
    for (int i = 0; i < RecognCount; ++i)
    {
        RecognChannel* recogn = new RecognChannel(i,ChannelCount/RecognCount, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, detectors);
        recogns->push_back(recogn);
    }
    for (int i = 0; i < ChannelCount; ++i)
    {
        DetectChannel* detect = new DetectChannel(i + 1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, recogns->at(i / (ChannelCount / RecognCount)), detectors[i]);
        detects->push_back(detect);
    }
}

void FlowStartup::InitDecodes()
{
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowChannelData data;
        FlowChannel channel = data.Get(i + 1);
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

string FlowStartup::GetChannelJson(const string& host,int channelIndex)
{
    string channelJson;
    FlowChannelData data;
    FlowChannel channel=data.Get(channelIndex);
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
        for (vector<FlowLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
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
            JsonSerialization::Serialize(&laneJson, "ioIp", lit->IOIp);
            JsonSerialization::Serialize(&laneJson, "ioPort", lit->IOPort);
            JsonSerialization::Serialize(&laneJson, "ioIndex", lit->IOIndex);
            JsonSerialization::Serialize(&laneJson, "detectLine", lit->DetectLine);
            JsonSerialization::Serialize(&laneJson, "stopLine", lit->StopLine);
            JsonSerialization::Serialize(&laneJson, "laneLine1", lit->LaneLine1);
            JsonSerialization::Serialize(&laneJson, "laneLine2", lit->LaneLine2);
            JsonSerialization::Serialize(&laneJson, "region", lit->Region);
            JsonSerialization::SerializeItem(&lanesJson, laneJson);
        }
        JsonSerialization::SerializeJsons(&channelJson, "lanes", lanesJson);
    }
    return channelJson;
}

void FlowStartup::SetDevice(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    int channelIndex = 0;
    vector<FlowChannel> channels;
    map<int, FlowChannel> tempChannels;
    while (true)
    {
        FlowChannel channel;
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
            FlowLane lane;
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
            lane.IOIp = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":ioIp"));
            lane.IOPort = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":ioPort"));
            lane.IOIndex = jd.Get<int>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":ioIndex"));
            lane.DetectLine = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":detectLine"));
            lane.StopLine = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":stopLine"));
            lane.LaneLine1 = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneLine1"));
            lane.LaneLine2 = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":laneLine2"));
            lane.Region = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":region"));
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
        tempChannels.insert(pair<int, FlowChannel>(channel.ChannelIndex, channel));
        channelIndex += 1;
    }
    FlowChannelData data;
    if (data.SetList(channels))
    {
        for (int i = 0; i < ChannelCount; ++i)
        {
            map<int, FlowChannel>::iterator it = tempChannels.find(i + 1);
            if (it == tempChannels.end())
            {
                DeleteDecode(i+1);
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

void FlowStartup::SetChannel(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);

    FlowChannel channel;
    channel.ChannelIndex = jd.Get<int>("channelIndex");
    channel.ChannelName = jd.Get<string>("channelName");
    channel.ChannelUrl = jd.Get<string>("channelUrl");
    channel.ChannelType = jd.Get<int>("channelType");
    int laneIndex = 0;
    while (true)
    {
        FlowLane lane;
        lane.LaneId = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":laneId"));
        if (lane.LaneId.empty())
        {
            break;
        }
        lane.ChannelIndex = channel.ChannelIndex;
        lane.LaneName = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":laneName"));
        lane.LaneIndex = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":laneIndex"));
        lane.LaneType = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":laneType"));
        lane.Direction = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":direction"));
        lane.FlowDirection = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":flowDirection"));
        lane.Length = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":length"));
        lane.IOIp = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":ioIp"));
        lane.IOPort = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":ioPort"));
        lane.IOIndex = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":ioIndex"));
        lane.DetectLine = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":detectLine"));
        lane.StopLine = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":stopLine"));
        lane.LaneLine1 = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":laneLine1"));
        lane.LaneLine2 = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":laneLine2"));
        lane.Region = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":region"));
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
    FlowChannelData data;
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

bool FlowStartup::DeleteChannel(int channelIndex)
{
    FlowChannelData data;
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
