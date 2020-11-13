#include "FlowStartup.h"

using namespace std;
using namespace OnePunchMan;

FlowStartup::FlowStartup()
    :TrafficStartup()
{
    JsonDeserialization jd("appsettings.json");
    TrafficDirectory::Init(jd.Get<string>("Flow:Web"));
    TrafficData::Init(jd.Get<string>("Flow:Db"));
    FlowDetector::Init(jd);
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

void FlowStartup::InitThreads(MqttChannel* mqtt, vector<DecodeChannel*>* decodes, vector<FrameHandler*>* handlers, vector<TrafficDetector*>* detectors)
{
    _merge = new DataMergeMap(mqtt);
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowDetector* detector = new FlowDetector(DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, mqtt,_merge);
        _detectors.push_back(detector);
        detectors->push_back(detector);
        DG_FrameHandler* handler=new DG_FrameHandler(i + 1,DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, detector);
        _handlers.push_back(handler);
        handlers->push_back(handler);
        DecodeChannel* decode = new DecodeChannel(i + 1, NULL, handler);
        decodes->push_back(decode);
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
            unsigned char taskId = _decodes[i]->UpdateChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), static_cast<ChannelType>(channel.ChannelType), channel.Loop);
            _handlers[i]->UpdateChannel(channel);
            _detectors[i]->UpdateChannel(taskId, channel);
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
            JsonSerialization::SerializeValue(&laneJson, "reportProperties", lit->ReportProperties);
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
            lane.ReportProperties = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":lanes:", laneIndex, ":reportProperties"));
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
                _handlers[i]->UpdateChannel(it->second);
                _detectors[i]->UpdateChannel(taskId,it->second);
            }
        }
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::BadRequest;
        JsonSerialization::SerializeArray(&e->ResponseJson, "db", data.LastError());
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
        lane.ReportProperties = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":reportProperties"));
        channel.Lanes.push_back(lane);
        laneIndex += 1;
    }
    string error = CheckFlowChannel(&channel);
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
        _handlers[channel.ChannelIndex - 1]->UpdateChannel(channel);
        _detectors[channel.ChannelIndex - 1]->UpdateChannel(taskId,channel);
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::BadRequest;
        JsonSerialization::SerializeArray(&e->ResponseJson, "db", data.LastError());
    }
}

string FlowStartup::CheckFlowChannel(FlowChannel* channel)
{
    string error = TrafficStartup::CheckChannel(channel);
    if (error.empty())
    {
        FlowChannelData data;
        string message;
        for (vector<FlowLane>::iterator it = channel->Lanes.begin(); it != channel->Lanes.end(); ++it)
        {
            vector<FlowLane> lanes=data.GetLaneList(channel->ChannelIndex, it->LaneId);
            for (vector<FlowLane>::iterator lit = lanes.begin(); lit != lanes.end(); ++lit)
            {
                if (it->ReportProperties & lit->ReportProperties)
                {
                    JsonSerialization::AddValueItem(&message, StringEx::Combine("channel:", it->ChannelIndex, " lane:", it->LaneId, " conflicts with channel:", lit->ChannelIndex, " lane:", lit->LaneId));
                }
            }
        }
        if (!message.empty())
        {
            string json;
            JsonSerialization::SerializeArray(&json, "lanes", message);
            return json;
        }
    }
    return error;
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