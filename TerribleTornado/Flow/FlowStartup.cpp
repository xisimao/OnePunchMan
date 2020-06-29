#include "FlowStartup.h"

using namespace std;
using namespace OnePunchMan;

FlowStartup::FlowStartup()
    :TrafficStartup()
{
    TrafficData::Init("flow.db");
}

FlowStartup::~FlowStartup()
{
    for (unsigned int i = 0; i < _detectors.size(); ++i)
    {
        delete _detectors[i];
    }
}

void FlowStartup::UpdateDb()
{
    FlowChannelData data;
    data.UpdateDb();
}

void FlowStartup::InitThreads(MqttChannel* mqtt, vector<HisiDecodeChannel*>* decodes, vector<TrafficDetector*>* detectors, vector<DetectChannel*>* detects, vector<RecognChannel*>* recogns)
{
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowDetector* detector = new FlowDetector(DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, mqtt);
        _detectors.push_back(detector);
        detectors->push_back(detector);
        HisiDecodeChannel* decode = new HisiDecodeChannel(i + 1,NULL);
        decodes->push_back(decode);
    }

    for (int i = 0; i < RecognCount; ++i)
    {
        RecognChannel* recogn = new RecognChannel(i, DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, detectors);
        recogns->push_back(recogn);
    }

    for (int i = 0; i < DetectCount; ++i)
    {
        DetectChannel* detect = new DetectChannel(i,DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight);
        detects->push_back(detect);
    }

    for (int i = 0; i < DetectCount; ++i)
    {
        detects->at(i)->SetRecogn(recogns->at(i%RecognCount));
        for (int j = 0; j < ChannelCount / DetectCount; ++j)
        {
            int channelIndex = i + (j * DetectCount) + 1;
            detects->at(i)->AddChannel(channelIndex, decodes->at(channelIndex-1), detectors->at(channelIndex-1));
        }
    }
}

void FlowStartup::InitChannels()
{
    FlowChannelData data;
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowChannel channel = data.Get(i + 1);
        if (!channel.ChannelUrl.empty())
        {
            unsigned char taskId= _decodes[i]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), channel.Loop);
            _detectors[i]->UpdateChannel(taskId,channel);
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
        JsonSerialization::SerializeValue(&channelJson, "channelIndex", channel.ChannelIndex);
        JsonSerialization::SerializeValue(&channelJson, "channelName", channel.ChannelName);
        JsonSerialization::SerializeValue(&channelJson, "channelUrl", channel.ChannelUrl);
        JsonSerialization::SerializeValue(&channelJson, "rtmpUrl", channel.RtmpUrl(host));
        JsonSerialization::SerializeValue(&channelJson, "channelType", channel.ChannelType);
        JsonSerialization::SerializeValue(&channelJson, "loop", channel.Loop);
        JsonSerialization::SerializeValue(&channelJson, "outputReport", channel.OutputReport);
        JsonSerialization::SerializeValue(&channelJson, "outputImage", channel.OutputImage);
        JsonSerialization::SerializeValue(&channelJson, "outputRecogn", channel.OutputRecogn);
        JsonSerialization::SerializeValue(&channelJson, "globalDetect", channel.GlobalDetect);
        if (ChannelIndexEnable(channel.ChannelIndex))
        {
            JsonSerialization::SerializeValue(&channelJson, "lanesInited", _detectors[channel.ChannelIndex - 1]->LanesInited());
        }

        string lanesJson;
        for (vector<FlowLane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
        {
            string laneJson;
            JsonSerialization::SerializeValue(&laneJson, "channelIndex", lit->ChannelIndex);
            JsonSerialization::SerializeValue(&laneJson, "laneId", lit->LaneId);
            JsonSerialization::SerializeValue(&laneJson, "laneName", lit->LaneName);
            JsonSerialization::SerializeValue(&laneJson, "laneIndex", lit->LaneIndex);
            JsonSerialization::SerializeValue(&laneJson, "laneType", lit->LaneType);
            JsonSerialization::SerializeValue(&laneJson, "direction", lit->Direction);
            JsonSerialization::SerializeValue(&laneJson, "flowDirection", lit->FlowDirection);
            JsonSerialization::SerializeValue(&laneJson, "length", lit->Length);
            JsonSerialization::SerializeValue(&laneJson, "ioIp", lit->IOIp);
            JsonSerialization::SerializeValue(&laneJson, "ioPort", lit->IOPort);
            JsonSerialization::SerializeValue(&laneJson, "ioIndex", lit->IOIndex);
            JsonSerialization::SerializeValue(&laneJson, "detectLine", lit->DetectLine);
            JsonSerialization::SerializeValue(&laneJson, "stopLine", lit->StopLine);
            JsonSerialization::SerializeValue(&laneJson, "laneLine1", lit->LaneLine1);
            JsonSerialization::SerializeValue(&laneJson, "laneLine2", lit->LaneLine2);
            JsonSerialization::SerializeValue(&laneJson, "region", lit->Region);
            JsonSerialization::AddClassItem(&lanesJson, laneJson);
        }
        JsonSerialization::SerializeArray(&channelJson, "lanes", lanesJson);
    }
    return channelJson;
}

string FlowStartup::CheckChannel(FlowChannel* channel)
{
    if (ChannelIndexEnable(channel->ChannelIndex))
    {
        if (channel->ChannelType != static_cast<int>(ChannelType::File)
            || channel->Loop)
        {
            channel->OutputImage = false;
            channel->OutputReport = false;
            channel->OutputRecogn = false;
            channel->GlobalDetect = false;
        }
        return string();
    }
    else
    {
        return GetErrorJson("channelIndex", StringEx::Combine("channelIndex is limited to 1-", ChannelCount));
    }
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
        string error = CheckChannel(&channel);
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
                _decodes[i]->ClearChannel();
                _detectors[i]->ClearChannel();
            }
            else
            {
                unsigned char taskId = _decodes[i]->UpdateChannel(it->second.ChannelUrl, it->second.RtmpUrl("127.0.0.1"), it->second.Loop);
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

void FlowStartup::SetChannel(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    FlowChannel channel;
    channel.ChannelIndex = jd.Get<int>("channelIndex");
    channel.ChannelName = jd.Get<string>("channelName");
    channel.ChannelUrl = jd.Get<string>("channelUrl");
    channel.ChannelType = jd.Get<int>("channelType");
    channel.Loop = jd.Get<bool>("loop");
    channel.OutputImage = jd.Get<bool>("outputImage");
    channel.OutputReport = jd.Get<bool>("outputReport");
    channel.OutputRecogn = jd.Get<bool>("outputRecogn");
    channel.GlobalDetect = jd.Get<bool>("globalDetect");
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
    string error = CheckChannel(&channel);
    if (!error.empty())
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = error;
        return;
    }
    FlowChannelData data;
    if (data.Set(channel))
    {
        unsigned char taskId = _decodes[channel.ChannelIndex - 1]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), channel.Loop);
        _detectors[channel.ChannelIndex - 1]->UpdateChannel(taskId,channel);
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
        _decodes[channelIndex - 1]->ClearChannel();
        _detectors[channelIndex - 1]->ClearChannel();
        return true;
    }
    else
    {
        return false;
    }
}

void FlowStartup::GetReport(int channelIndex,HttpReceivedEventArgs* e)
{
    _detectors[channelIndex-1]->GetReportJson(&e->ResponseJson);
}