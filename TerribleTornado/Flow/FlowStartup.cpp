#include "FlowStartup.h"

using namespace std;
using namespace OnePunchMan;

FlowStartup::FlowStartup()
    :TrafficStartup()
{
}

FlowStartup::~FlowStartup()
{
    for (unsigned int i = 0; i < _detectors.size(); ++i)
    {
        delete _detectors[i];
    }
}

void FlowStartup::Update(HttpReceivedEventArgs* e)
{
    if (UrlStartWith(e->Url, "/api/report"))
    {
        string id = GetId(e->Url, "/api/report");
        int channelIndex = StringEx::Convert<int>(id);
        if (ChannelIndexEnable(channelIndex))
        {
            GetReport(channelIndex, e);
            e->Code = HttpCode::OK;
        }
        else
        {
            e->Code = HttpCode::NotFound;
        }
    }
    else
    {
        TrafficStartup::Update(e);
    }
}

void FlowStartup::UpdateDb()
{
    FlowChannelData data;
    data.UpdateDb();
}

void FlowStartup::InitThreads(MqttChannel* mqtt, vector<DecodeChannel*>* decodes, vector<TrafficDetector*>* detectors, vector<DetectChannel*>* detects, vector<RecognChannel*>* recogns, int loginHandler)
{
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowDetector* detector = new FlowDetector(DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, mqtt);
        _detectors.push_back(detector);
        detectors->push_back(detector);
        DecodeChannel* decode = new DecodeChannel(i + 1, loginHandler, NULL);
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
            unsigned char taskId= _decodes[i]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), static_cast<ChannelType>(channel.ChannelType), channel.Loop);
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
        FillChannelJson(&channelJson, &channel, host);
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
            JsonSerialization::SerializeValue(&laneJson, "region", lit->Region);
            JsonSerialization::AddClassItem(&lanesJson, laneJson);
        }
        JsonSerialization::SerializeArray(&channelJson, "lanes", lanesJson);
    }
    return channelJson;
}

void FlowStartup::SetDevice(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    int itemIndex = 0;
    vector<FlowChannel> channels;
    map<int, FlowChannel> tempChannels;
    while (true)
    {
        FlowChannel channel;
        FillChannel(&channel, jd, itemIndex);
        if (channel.ChannelIndex == 0)
        {
            break;
        }
        int laneIndex = 0;
        while (true)
        {
            FlowLane lane;
            lane.LaneId = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":laneId"));
            if (lane.LaneId.empty())
            {
                break;
            }
            lane.ChannelIndex = channel.ChannelIndex;
            lane.LaneName = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":laneName"));
            lane.LaneIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":laneIndex"));
            lane.LaneType = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":laneType"));
            lane.Direction = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":direction"));
            lane.FlowDirection = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":flowDirection"));
            lane.Length = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":length"));
            lane.IOIp = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":ioIp"));
            lane.IOPort = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":ioPort"));
            lane.IOIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":ioIndex"));
            lane.DetectLine = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":detectLine"));
            lane.StopLine = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":stopLine"));
            lane.Region = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":region"));
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
        itemIndex += 1;
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
                unsigned char taskId = _decodes[i]->UpdateChannel(it->second.ChannelUrl, it->second.RtmpUrl("127.0.0.1"), static_cast<ChannelType>(it->second.ChannelType), it->second.Loop);
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
    FillChannel(&channel, jd);
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
        unsigned char taskId = _decodes[channel.ChannelIndex - 1]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), static_cast<ChannelType>(channel.ChannelType), channel.Loop);
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