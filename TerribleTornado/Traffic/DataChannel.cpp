#include "DataChannel.h"

using namespace std;
using namespace Saitama;
using namespace TerribleTornado;

const string DataChannel::FlowTopic("Flow");

DataChannel::DataChannel()
    :ThreadObject("data"), _socketMaid(NULL),_mqtt(NULL)
{
    DecodeChannel::InitFFmpeg();
    DecodeChannel::UninitHisi(FlowChannelData::ChannelCount);
    if (!DecodeChannel::InitHisi(FlowChannelData::ChannelCount))
    {
        _inited = false;
        return;
    }
    if (!SeemmoSDK::Init())
    {
        _inited = false;
        return;
    }

    _socketMaid=new SocketMaid(2);
    _handler.HttpReceived.Subscribe(this);
    if (_socketMaid->AddListenEndPoint(EndPoint(7772), &_handler)==-1)
    {
        _inited = false;
        return;
    }
    _inited = true;
    _mqtt = new MqttChannel("127.0.0.1", 1883);
    _mqtt->Start();


    for (int i = 0; i < FlowChannelData::ChannelCount; ++i)
    {
        _decodes.push_back(NULL);
        ChannelDetector* detector = new ChannelDetector(DecodeChannel::VideoWidth, DecodeChannel::VideoHeight, _mqtt);
        _detectors.push_back(detector);
    }

    for (int i = 0; i < FlowChannelData::ChannelCount; ++i)
    {
        if (i % RecognChannel::ItemCount == 0)
        {
            RecognChannel* recogn = new RecognChannel(i / RecognChannel::ItemCount, DecodeChannel::VideoWidth, DecodeChannel::VideoHeight, _detectors);
            _recogns.push_back(recogn);
            recogn->Start();
        }
        DetectChannel* detect = new DetectChannel(i + 1, DecodeChannel::VideoWidth, DecodeChannel::VideoHeight, _recogns[i / RecognChannel::ItemCount], _detectors[i]);
        _detects.push_back(detect);
        detect->Start();
    }

    FlowChannelData data;
    vector<FlowChannel> channels = data.GetList();
    for (unsigned int i = 0; i < channels.size(); ++i)
    {
        SetChannel(channels[i]);
    }
    _socketMaid->Start();
    _startTime = DateTime::Now();
    _lastMinute = _startTime.Minute();
}

DataChannel::~DataChannel()
{
    if (_socketMaid != NULL)
    {
        _socketMaid->Stop();
    }

    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        if (_decodes[i] != NULL)
        {
            _decodes[i]->Stop();
            delete _decodes[i];
        }
    }

    for (unsigned int i = 0; i < _detects.size(); ++i)
    {
        _detects[i]->Stop();
        delete _detects[i];
    }
    for (unsigned int i = 0; i < _recogns.size(); ++i)
    {
        _recogns[i]->Stop();
        delete _recogns[i];
    }
    for (unsigned int i = 0; i < _detectors.size(); ++i)
    {
        delete _detectors[i];
    }
    if (_mqtt != NULL)
    {
        _mqtt->Stop();
    }  
    SeemmoSDK::Uninit();
    DecodeChannel::UninitHisi(FlowChannelData::ChannelCount);
    DecodeChannel::UninitFFmpeg();
}

void DataChannel::Update(HttpReceivedEventArgs* e)
{
    if (UrlStartWith(e->Url, "/api/device"))
    {
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            GetDevice(e);
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            SetDevice(e);
        }
    }
    else if (UrlStartWith(e->Url, "/api/channels"))
    {
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            GetChannel(e);
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            SetChannel(e);
        }
        else if (e->Function.compare(HttpFunction::Delete) == 0)
        {
            DeleteChannel(e);
        }
    }
    else if (UrlStartWith(e->Url, "/api/system"))
    {
        Stop();
        LogPool::Information(LogEvent::Flow, "exit system");
        e->Code = HttpCode::OK;
    }
}

void DataChannel::GetDevice(HttpReceivedEventArgs* e)
{
    FlowDevice device;
    device.LicenceStatus = SeemmoSDK::Inited;
    device.SoftwareVersion = "1.0.0";
    device.SN = StringEx::Trim(Command::Execute("cat /mtd/basesys/data/devsn"));
    string df = Command::Execute("df");
    vector<string> rows = StringEx::Split(df, "\n", true);
    for (unsigned int i = 0; i < rows.size(); ++i)
    {
        vector<string> columns = StringEx::Split(rows[i], " ", true);
        if (columns.size() >= 6 && columns[columns.size() - 1].compare("/") == 0)
        {
            long long used = StringEx::Convert<long long>(columns[2]);
            long long total = used + StringEx::Convert<long long>(columns[3]);
            device.DiskUsed = StringEx::ToString(StringEx::Rounding(static_cast<double>(used) / 1024.0 / 1024.0, 2));
            device.DiskTotal = StringEx::ToString(StringEx::Rounding(static_cast<double>(total) / 1024.0 / 1024.0, 2));
            break;
        }
    }
    FlowChannelData data;
    vector<FlowChannel> channels = data.GetList();
    string channelsJson;
    for (unsigned int i=0;i<channels.size();++i)
    {
        string channelJson = GetChannelJson(e,channels[i]);
        JsonSerialization::SerializeItem(&channelsJson, channelJson);
    }

    string deviceJson;
    DateTime now = DateTime::UtcNow();
    JsonSerialization::Serialize(&deviceJson, "licenceStatus", device.LicenceStatus);
    JsonSerialization::Serialize(&deviceJson, "startTime", _startTime.ToString());
    JsonSerialization::Serialize(&deviceJson, "deviceTime", now.TimeStamp());
    JsonSerialization::Serialize(&deviceJson, "deviceTime_Desc", now.ToString());
    JsonSerialization::Serialize(&deviceJson, "softwareVersion", device.SoftwareVersion);
    JsonSerialization::Serialize(&deviceJson, "sn", device.SN);
    JsonSerialization::Serialize(&deviceJson, "diskUsed", device.DiskUsed);
    JsonSerialization::Serialize(&deviceJson, "diskTotal", device.DiskTotal);
    JsonSerialization::Serialize(&deviceJson, "videoWidth", DecodeChannel::VideoWidth);
    JsonSerialization::Serialize(&deviceJson, "videoHeight", DecodeChannel::VideoHeight);
    JsonSerialization::SerializeJson(&deviceJson, "channels", channelsJson);

    e->Code = HttpCode::OK;
    e->ResponseJson = deviceJson;
}

void DataChannel::SetDevice(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    int channelIndex = 0;
    vector<FlowChannel> channels;
    map<int,FlowChannel> tempChannels;
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
            lane.Region = jd.Get<string>(StringEx::Combine("channels:", channelIndex, ":lanes:", laneIndex, ":region"));
            channel.Lanes.push_back(lane);
            laneIndex += 1;
        }
        string error = CheckChannel(channel);
        if (!error.empty())
        {
            e->Code = HttpCode::BadRequest;
            e->ResponseJson = error;
            return;
        }
        channels.push_back(channel);
        tempChannels.insert(pair<int,FlowChannel>(channel.ChannelIndex, channel));
        channelIndex += 1;
    }
    FlowChannelData data;
    if (data.SetList(channels))
    {
        for (int i = 0; i < FlowChannelData::ChannelCount; ++i)
        {
            if (tempChannels.find(i + 1) == tempChannels.end())
            {
                DeleteChannel(i+1);
            }
            else
            {
                SetChannel(tempChannels[i+1]);
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

void DataChannel::GetChannel(Saitama::HttpReceivedEventArgs* e)
{
    string id = GetId(e->Url, "/api/channels");
    int channelIndex = StringEx::Convert<int>(id);
    FlowChannelData data;
    FlowChannel channel = data.Get(channelIndex);
    if (channel.ChannelIndex == 0)
    {
        e->Code = HttpCode::NotFound;
    }
    else
    {
        e->ResponseJson = GetChannelJson(e, channel);
        e->Code = HttpCode::OK;
    }
}

void DataChannel::SetChannel(Saitama::HttpReceivedEventArgs* e)
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
        Lane lane;
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
        lane.IOIp = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":iOIp"));
        lane.IOPort = jd.Get<int>(StringEx::Combine("lanes:", laneIndex, ":iOPort"));
        lane.IOIndex = jd.Get<int>(StringEx::Combine("clanes:", laneIndex, ":iOIndex"));
        lane.DetectLine = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":detectLine"));
        lane.StopLine = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":stopLine"));
        lane.LaneLine1 = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":laneLine1"));
        lane.LaneLine2 = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":laneLine2"));
        lane.Region = jd.Get<string>(StringEx::Combine("lanes:", laneIndex, ":region"));
        channel.Lanes.push_back(lane);
        laneIndex += 1;
    }
    string error = CheckChannel(channel);
    if (!error.empty())
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = error;
        return;
    }
    FlowChannelData data;
    if (data.Set(channel))
    {
        SetChannel(channel);
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = GetErrorJson("db", data.LastError());
    }
}

void DataChannel::DeleteChannel(Saitama::HttpReceivedEventArgs* e)
{
    string id = GetId(e->Url, "/api/channels");
    int channelIndex = StringEx::Convert<int>(id);
    FlowChannelData data;
    if (data.Delete(channelIndex))
    {
        DeleteChannel(channelIndex);
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::NotFound;
    }

}

void DataChannel::SetChannel(const FlowChannel& channel)
{
    lock_guard<mutex> lck(_decodeMutex);
    if (ChannelIndexEnable(channel.ChannelIndex))
    {
        //如果地址不一致
        if (_detectors[channel.ChannelIndex - 1]->ChannelUrl().compare(channel.ChannelUrl) != 0)
        {
            if (_decodes[channel.ChannelIndex - 1] != NULL)
            {
                _decodes[channel.ChannelIndex - 1]->Stop();
                delete _decodes[channel.ChannelIndex - 1];
            }
            if (channel.ChannelType == static_cast<int>(ChannelType::RTSP))
            {
                DecodeChannel* decode = new DecodeChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), false, channel.ChannelIndex, _detects[channel.ChannelIndex - 1]);
                decode->Start();
                _decodes[channel.ChannelIndex - 1] = decode;
            }
            else if (channel.ChannelType == static_cast<int>(ChannelType::File))
            {
                DecodeChannel* decode = new DecodeChannel(channel.ChannelUrl, channel.RtmpUrl("127.0.0.1"), true, channel.ChannelIndex, _detects[channel.ChannelIndex - 1]);
                decode->Start();
                _decodes[channel.ChannelIndex - 1] = decode;
            }
            else
            {
                _decodes[channel.ChannelIndex - 1] = NULL;
            }
        }
        _detectors[channel.ChannelIndex - 1]->UpdateChannel(channel);
   }
}

void DataChannel::DeleteChannel(int channelIndex)
{
    lock_guard<mutex> lck(_decodeMutex);
    if (ChannelIndexEnable(channelIndex))
    {
        if (_decodes[channelIndex-1] != NULL)
        {
            _decodes[channelIndex - 1]->Stop();
            delete _decodes[channelIndex - 1];
            _decodes[channelIndex - 1] = NULL;
        }
        _detectors[channelIndex - 1]->ClearChannel();
    }
}

string DataChannel::CheckChannel(const FlowChannel& channel)
{
    if (ChannelIndexEnable(channel.ChannelIndex))
    {
        return string();
    }
    else
    {
        return GetErrorJson("channelIndex", StringEx::Combine("channelIndex is limited to 1-", FlowChannelData::ChannelCount));
    }
}

string DataChannel::GetChannelJson(Saitama::HttpReceivedEventArgs* e, const FlowChannel& channel)
{
    string channelJson;
    JsonSerialization::Serialize(&channelJson, "channelIndex", channel.ChannelIndex);
    JsonSerialization::Serialize(&channelJson, "channelName", channel.ChannelName);
    JsonSerialization::Serialize(&channelJson, "channelUrl", channel.ChannelUrl);
    JsonSerialization::Serialize(&channelJson, "rtmpUrl", channel.RtmpUrl(EndPoint::GetLocalEndPoint(e->Socket).HostIp()));
    JsonSerialization::Serialize(&channelJson, "channelType", channel.ChannelType);
    if (ChannelIndexEnable(channel.ChannelIndex))
    {
        lock_guard<mutex> lck(_decodeMutex);
        JsonSerialization::Serialize(&channelJson, "channelStatus", _decodes[channel.ChannelIndex - 1] == NULL ? 0 : static_cast<int>(_decodes[channel.ChannelIndex - 1]->Status()));
    }
    string lanesJson;
    for (vector<Lane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
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
        JsonSerialization::Serialize(&laneJson, "region", lit->Region);
        JsonSerialization::SerializeItem(&lanesJson, laneJson);
    }
    JsonSerialization::SerializeJsons(&channelJson, "lanes", lanesJson);

    return channelJson;

}

bool DataChannel::ChannelIndexEnable(int channelIndex)
{
    return channelIndex >= 1 && channelIndex <= FlowChannelData::ChannelCount;
}

bool DataChannel::UrlStartWith(const string& url, const string& key)
{
    if (url.size() == key.size())
    {
        return url.compare(key) == 0;
    }
    else if(url.size() >= key.size())
    {
        return url.substr(0, key.size()).compare(key) == 0
            &&url[key.size()]=='/';
    }
    else
    {
        return false;
    }
}

string DataChannel::GetId(const std::string& url, const std::string& key)
{
    return url.size() >= key.size() ? url.substr(key.size()+1, url.size() - key.size()-1) : string();
}

string DataChannel::GetErrorJson(const string& field,const string& message)
{
    return StringEx::Combine("{\"", field, "\":[\"", message, "\"]}");
}

void DataChannel::StartCore()
{
    if (!_inited)
    {
        LogPool::Information(LogEvent::Flow, "init flow system failed exit");
        return;
    }
    while (!_cancelled)
    {
        this_thread::sleep_for(chrono::seconds(1));

        //收集流量
        DateTime now = DateTime::Now();
        int currentMinute = now.Minute();
        if (currentMinute != _lastMinute)
        {
            _lastMinute = currentMinute;
            long long timeStamp = DateTime(now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), 0).TimeStamp();
            string json;
            for (unsigned int i = 0; i < _detectors.size(); ++i)
            {
                _detectors[i]->CollectFlow(&json,timeStamp);
            }
            if (!json.empty())
            {
                _mqtt->Send(FlowTopic, json);
            }
        }
    }
}