#include "TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

const int TrafficStartup::ChannelCount = 8;

TrafficStartup::TrafficStartup()
    :ThreadObject("data"),_mqtt(NULL), _startTime(DateTime::Now()), _sdkInited(false), _socketMaid(NULL)
{

}

TrafficStartup::~TrafficStartup()
{
    if (_socketMaid != NULL)
    {
        delete _socketMaid;
    }
    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        if (_decodes[i] != NULL)
        {
            delete _decodes[i];
        }
    }
    for (unsigned int i = 0; i < _detects.size(); ++i)
    {
        delete _detects[i];
    }
    for (unsigned int i = 0; i < _recogns.size(); ++i)
    {
        delete _recogns[i];
    }
    if (_mqtt != NULL)
    {
        delete _mqtt;
    }
    SeemmoSDK::Uninit();
    DecodeChannel::UninitHisi(ChannelCount);
    FFmpegChannel::UninitFFmpeg();
}

bool TrafficStartup::Init()
{
    FFmpegChannel::InitFFmpeg();
    DecodeChannel::UninitHisi(ChannelCount);
    if (!DecodeChannel::InitHisi(ChannelCount))
    {
        return false;
    }

    _socketMaid = new SocketMaid(2);
    _handler.HttpReceived.Subscribe(this);
    if (_socketMaid->AddListenEndPoint(EndPoint(7772), &_handler) == -1)
    {
        return false;
    }

    _sdkInited = SeemmoSDK::Init();

    _mqtt = new MqttChannel("127.0.0.1", 1883);

    for (int i = 0; i < ChannelCount; ++i)
    {
        _decodes.push_back(NULL);
    }

    vector<TrafficDetector*> detectors= InitDetectors();

    for (int i = 0; i < ChannelCount; ++i)
    {
        if (i % RecognChannel::ItemCount == 0)
        {
            RecognChannel* recogn = new RecognChannel(i / RecognChannel::ItemCount, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, detectors);
            _recogns.push_back(recogn);
        }
        DetectChannel* detect = new DetectChannel(i + 1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, _recogns[i / RecognChannel::ItemCount], detectors[i]);
        _detects.push_back(detect);
    }

    return true;
}

void TrafficStartup::Update(HttpReceivedEventArgs* e)
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
            string id = GetId(e->Url, "/api/channels");
            int channelIndex = StringEx::Convert<int>(id);
            string channelJson = GetChannelJson(e->Host,channelIndex);
            if (channelJson.empty())
            {
                lock_guard<mutex> lck(_decodeMutex);
                JsonSerialization::Serialize(&channelJson, "channelStatus", _decodes[channelIndex-1] == NULL ? 0 : static_cast<int>(_decodes[channelIndex - 1]->Status()));
                JsonSerialization::Serialize(&channelJson, "frameSpan", _decodes[channelIndex - 1] == NULL ? 0 : _decodes[channelIndex - 1]->PacketSpan());
                JsonSerialization::Serialize(&channelJson, "sourceWidth", _decodes[channelIndex - 1] == NULL ? 0 : _decodes[channelIndex - 1]->SourceWidth());
                JsonSerialization::Serialize(&channelJson, "sourceHeight", _decodes[channelIndex - 1] == NULL ? 0 : _decodes[channelIndex - 1]->SourceHeight());
                e->Code = HttpCode::NotFound;
            }
            else
            {
                e->ResponseJson = channelJson;
                e->Code = HttpCode::OK;
            }
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            SetChannel(e);
        }
        else if (e->Function.compare(HttpFunction::Delete) == 0)
        {
            string id = GetId(e->Url, "/api/channels");
            int channelIndex = StringEx::Convert<int>(id);
            if (DeleteChannel(channelIndex))
            {
                e->Code = HttpCode::OK;
            }
            else
            {
                e->Code = HttpCode::NotFound;
            }
        }
    }
    else if (UrlStartWith(e->Url, "/api/system"))
    {
        Stop();
        LogPool::Information(LogEvent::Flow, "exit system");
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/upload"))
    {
        vector<string> requestLines = StringEx::Split(e->RequestJson, "\r\n");
        if (requestLines.size() >= 5)
        {
            FILE* file = fopen("/mtd/seemmo/programs/aisdk/data/licence","wb");
            if (file != NULL)
            {
                fwrite(requestLines[4].c_str(), 1, requestLines[4].size(), file);
                fclose(file);
            }
        }
        e->Code = HttpCode::OK;
    }
}

void TrafficStartup::GetDevice(HttpReceivedEventArgs* e)
{
    string softwareVersion = "1.0.0";
    string sn = StringEx::Trim(Command::Execute("cat /mtd/basesys/data/devguid"));
    string df = Command::Execute("df");
    string diskUsed;
    string diskTotal;
    vector<string> rows = StringEx::Split(df, "\n", true);
    for (unsigned int i = 0; i < rows.size(); ++i)
    {
        vector<string> columns = StringEx::Split(rows[i], " ", true);
        if (columns.size() >= 6 && columns[columns.size() - 1].compare("/") == 0)
        {
            long long used = StringEx::Convert<long long>(columns[2]);
            long long total = used + StringEx::Convert<long long>(columns[3]);
            diskUsed = StringEx::ToString(StringEx::Rounding(static_cast<double>(used) / 1024.0 / 1024.0, 2));
            diskTotal = StringEx::ToString(StringEx::Rounding(static_cast<double>(total) / 1024.0 / 1024.0, 2));
            break;
        }
    }

    string channelsJson;
    for (unsigned int i = 0; i < ChannelCount; ++i)
    {
        string channelJson = GetChannelJson(e->Host,i+1);
        if (!channelJson.empty()) 
        {
            lock_guard<mutex> lck(_decodeMutex);
            JsonSerialization::Serialize(&channelJson, "channelStatus", _decodes[i] == NULL ? 0 : static_cast<int>(_decodes[i]->Status()));
            JsonSerialization::Serialize(&channelJson, "frameSpan", _decodes[i] == NULL ? 0 : _decodes[i]->PacketSpan());
            JsonSerialization::Serialize(&channelJson, "sourceWidth", _decodes[i] == NULL ? 0 : _decodes[i]->SourceWidth());
            JsonSerialization::Serialize(&channelJson, "sourceHeight", _decodes[i] == NULL ? 0 : _decodes[i]->SourceHeight());
            JsonSerialization::SerializeItem(&channelsJson, channelJson);
        }
    }
 
    string deviceJson;
    DateTime now = DateTime::Now();
    JsonSerialization::Serialize(&deviceJson, "deviceTime", now.UtcTimeStamp());
    JsonSerialization::Serialize(&deviceJson, "deviceTime_Desc", now.ToString());
    JsonSerialization::Serialize(&deviceJson, "startTime", _startTime.ToString());
    JsonSerialization::Serialize(&deviceJson, "diskUsed", diskUsed);
    JsonSerialization::Serialize(&deviceJson, "diskTotal", diskTotal);
    JsonSerialization::Serialize(&deviceJson, "licenceStatus", _sdkInited);
    JsonSerialization::Serialize(&deviceJson, "sn", sn);
    JsonSerialization::Serialize(&deviceJson, "softwareVersion", softwareVersion);
    JsonSerialization::Serialize(&deviceJson, "destinationWidth", FFmpegChannel::DestinationWidth);
    JsonSerialization::Serialize(&deviceJson, "destinationHeight", FFmpegChannel::DestinationHeight);
    JsonSerialization::Serialize(&deviceJson, "mqttConnected", _mqtt->Connected());
    JsonSerialization::SerializeJson(&deviceJson, "channels", channelsJson);

    e->Code = HttpCode::OK;
    e->ResponseJson = deviceJson;
}


void TrafficStartup::SetDecode(int channelIndex, const string& inputUrl, const string& outputUrl)
{
    lock_guard<mutex> lck(_decodeMutex);
    if (ChannelIndexEnable(channelIndex))
    {
        if (_decodes[channelIndex - 1] == NULL)
        {
            DecodeChannel* decode = new DecodeChannel(inputUrl, outputUrl, channelIndex, _detects[channelIndex - 1]);
            decode->Start();
            _decodes[channelIndex - 1] = decode;
        }
        else
        {
            //如果地址不一致
            if (_decodes[channelIndex - 1]->InputUrl().compare(inputUrl) != 0)
            {
                _decodes[channelIndex - 1]->Stop();
                delete _decodes[channelIndex - 1];
                DecodeChannel* decode = new DecodeChannel(inputUrl, outputUrl, channelIndex, _detects[channelIndex - 1]);
                decode->Start();
                _decodes[channelIndex - 1] = decode;
            }
        }
    }
}

void TrafficStartup::DeleteDecode(int channelIndex)
{
    lock_guard<mutex> lck(_decodeMutex);
    if (ChannelIndexEnable(channelIndex))
    {
        if (_decodes[channelIndex - 1] != NULL)
        {
            _decodes[channelIndex - 1]->Stop();
            delete _decodes[channelIndex - 1];
            _decodes[channelIndex - 1] = NULL;
        }
    }
}

string TrafficStartup::CheckChannel(int channelIndex)
{
    if (ChannelIndexEnable(channelIndex))
    {
        return string();
    }
    else
    {
        return GetErrorJson("channelIndex", StringEx::Combine("channelIndex is limited to 1-", ChannelCount));
    }
}

bool TrafficStartup::ChannelIndexEnable(int channelIndex)
{
    return channelIndex >= 1 && channelIndex <= ChannelCount;
}

bool TrafficStartup::UrlStartWith(const string& url, const string& key)
{
    if (url.size() == key.size())
    {
        return url.compare(key) == 0;
    }
    else if (url.size() >= key.size())
    {
        return url.substr(0, key.size()).compare(key) == 0
            && url[key.size()] == '/';
    }
    else
    {
        return false;
    }
}

string TrafficStartup::GetId(const std::string& url, const std::string& key)
{
    return url.size() >= key.size() ? url.substr(key.size() + 1, url.size() - key.size() - 1) : string();
}

string TrafficStartup::GetErrorJson(const string& field, const string& message)
{
    return StringEx::Combine("{\"", field, "\":[\"", message, "\"]}");
}

void TrafficStartup::StartCore()
{
    if (_sdkInited)
    {
        for (unsigned int i = 0; i < _detects.size(); ++i)
        {
            _detects[i]->Start();
        }
        for (unsigned int i = 0; i < _recogns.size(); ++i)
        {
            _recogns[i]->Start();
        }
    }
    //sdk未授权只启动网络上传功能
    else
    {
        if (_socketMaid != NULL)
        {
            _socketMaid->Start();
        }
    }

    bool sdkReady = false;
    while (!_cancelled)
    {
        this_thread::sleep_for(chrono::seconds(1));
        if (sdkReady)
        {
           PollCore();
        }
        else
        {
            sdkReady = true;
#ifndef _WIN32
            for (unsigned int i = 0; i < _detects.size(); ++i)
            {
                if (_detects[i]->IsBusy())
                {
                    sdkReady = false;
                }
            }
#endif // !_WIN32

            if (sdkReady)
            {
                InitChannels();
                if (_mqtt != NULL)
                {
                    _mqtt->Start();
                }
                if (_socketMaid != NULL)
                {
                    _socketMaid->Start();
                }
            }
        }   
    }

    if (_socketMaid != NULL)
    {
        _socketMaid->Stop();
    }

    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        if (_decodes[i] != NULL)
        {
            _decodes[i]->Stop();
        }
    }

    for (unsigned int i = 0; i < _detects.size(); ++i)
    {
        _detects[i]->Stop();
    }
    for (unsigned int i = 0; i < _recogns.size(); ++i)
    {
        _recogns[i]->Stop();
    }

    if (_mqtt != NULL)
    {
        _mqtt->Stop();
    }
}