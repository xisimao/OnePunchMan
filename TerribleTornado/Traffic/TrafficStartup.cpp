#include "TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

const int TrafficStartup::ChannelCount = 8;
const int TrafficStartup::DetectCount = 4;
const int TrafficStartup::RecognCount = 2;

TrafficStartup::TrafficStartup()
    :_startTime(DateTime::Now()), _sdkInited(false), _socketMaid(NULL), _mqtt(NULL)
{

}

void TrafficStartup::Update(MqttDisconnectedEventArgs* e)
{
    LogPool::Information(LogEvent::System,"system restart");
    exit(1);
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
            string channelJson = GetChannelJson(e->Host, channelIndex);
            if (channelJson.empty())
            {
                e->Code = HttpCode::NotFound;
            }
            else
            {
                JsonSerialization::SerializeValue(&channelJson, "channelStatus", static_cast<int>(_decodes[channelIndex - 1]->Status()));
                JsonSerialization::SerializeValue(&channelJson, "frameSpan",  _decodes[channelIndex - 1]->FrameSpan());
                JsonSerialization::SerializeValue(&channelJson, "sourceWidth", _decodes[channelIndex - 1]->SourceWidth());
                JsonSerialization::SerializeValue(&channelJson, "sourceHeight",  _decodes[channelIndex - 1]->SourceHeight());
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
    else if (UrlStartWith(e->Url, "/api/images"))
    {
        string id = GetId(e->Url, "/api/images");
        int channelIndex = StringEx::Convert<int>(id);
        if (ChannelIndexEnable(channelIndex))
        {
            _decodes[channelIndex - 1]->WriteBmp();
            e->Code = HttpCode::OK;
        }
        else
        {
            e->Code = HttpCode::NotFound;
        }
    }
    else if (UrlStartWith(e->Url, "/api/update/licence"))
    {
        LogPool::Information(LogEvent::Http, "update licence");
        string filePath("/mtd/seemmo/programs/aisdk/data/licence");
        HttpHandler::WriteFile(e->RequestJson, filePath);
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/update/system"))
    {
        LogPool::Information(LogEvent::Http, "update system");
        string filePath = Path::Combine(Path::GetCurrentPath(), "service.tar");
        HttpHandler::WriteFile(e->RequestJson, filePath);
        Command::Execute("tar xf service.tar -C ../../");
        Command::Execute("rm service.tar");
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/logs/export"))
    {
        string ls = Command::Execute("cd ../logs;tar cf logs.tar *.log");
        e->ResponseJson = "logs.tar";
        e->Code = HttpCode::OK;
    }
}

void TrafficStartup::GetDevice(HttpReceivedEventArgs* e)
{
    string sn = StringEx::Trim(Command::Execute("cat /mtd/basesys/data/devguid"));
    string df = Command::Execute("df");
    string diskUsed;
    string diskTotal;
    vector<string> dfRows = StringEx::Split(df, "\n", true);
    for (unsigned int i = 0; i < dfRows.size(); ++i)
    {
        vector<string> columns = StringEx::Split(dfRows[i], " ", true);
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
        string channelJson = GetChannelJson(e->Host, i + 1);
        if (!channelJson.empty())
        {
            JsonSerialization::SerializeValue(&channelJson, "channelStatus", static_cast<int>(_decodes[i]->Status()));
            JsonSerialization::SerializeValue(&channelJson, "frameSpan", _decodes[i]->FrameSpan());
            JsonSerialization::SerializeValue(&channelJson, "sourceWidth", _decodes[i]->SourceWidth());
            JsonSerialization::SerializeValue(&channelJson, "sourceHeight", _decodes[i]->SourceHeight());
            JsonSerialization::AddClassItem(&channelsJson, channelJson);
        }
    }

    string deviceJson;
    DateTime now = DateTime::Now();
    JsonSerialization::SerializeValue(&deviceJson, "deviceTime", now.UtcTimeStamp());
    JsonSerialization::SerializeValue(&deviceJson, "deviceTime_Desc", now.ToString());
    JsonSerialization::SerializeValue(&deviceJson, "startTime", _startTime.ToString());
    JsonSerialization::SerializeValue(&deviceJson, "diskUsed", diskUsed);
    JsonSerialization::SerializeValue(&deviceJson, "diskTotal", diskTotal);
    JsonSerialization::SerializeValue(&deviceJson, "licenceStatus", _sdkInited);
    JsonSerialization::SerializeValue(&deviceJson, "sn", sn);
    JsonSerialization::SerializeValue(&deviceJson, "softwareVersion", _softwareVersion);
    JsonSerialization::SerializeValue(&deviceJson, "webVersion", _webVersion);
    JsonSerialization::SerializeValue(&deviceJson, "sdkVersion", _sdkVersion);
    JsonSerialization::SerializeValue(&deviceJson, "destinationWidth", FFmpegChannel::DestinationWidth);
    JsonSerialization::SerializeValue(&deviceJson, "destinationHeight", FFmpegChannel::DestinationHeight);
    JsonSerialization::SerializeValue(&deviceJson, "mqttConnected", _mqtt==NULL?0:static_cast<int>(_mqtt->Status()));
    for (unsigned int i = 0; i < _recogns.size(); ++i)
    {
        JsonSerialization::SerializeValue(&deviceJson, StringEx::Combine("recognQueue", i + 1), _recogns[i]->Size());
    }
    JsonSerialization::SerializeClass(&deviceJson, "channels", channelsJson);
    e->Code = HttpCode::OK;
    e->ResponseJson = deviceJson;
}

void TrafficStartup::SetDecode(int channelIndex, const string& inputUrl, const string& outputUrl)
{
    if (ChannelIndexEnable(channelIndex))
    {
        _decodes[channelIndex - 1]->UpdateChannel(inputUrl, outputUrl);
    }
}

void TrafficStartup::DeleteDecode(int channelIndex)
{
    if (ChannelIndexEnable(channelIndex))
    {
        _decodes[channelIndex - 1]->ClearChannel();
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

DetectChannel* TrafficStartup::GetDetect(int channelIndex)
{
    if (ChannelIndexEnable(channelIndex))
    {
        return _detects[(channelIndex - 1) / (ChannelCount / DetectCount)];
    }
    else
    {
        return NULL;
    }
}

void TrafficStartup::Startup()
{
    Socket::Init();
    MqttChannel::Init();
    FFmpegChannel::InitFFmpeg();
    DecodeChannel::UninitHisi(ChannelCount);
    if (!DecodeChannel::InitHisi(ChannelCount))
    {
        exit(2);
    }

    _socketMaid = new SocketMaid(2,false);
    _handler.HttpReceived.Subscribe(this);
    if (_socketMaid->AddListenEndPoint(EndPoint(7772), &_handler) == -1)
    {
        exit(2);
    }

    //初始化sdk
    _sdkInited = SeemmoSDK::Init();
    if (SeemmoSDK::seemmo_version != NULL)
    {
        _sdkVersion = SeemmoSDK::seemmo_version();
    }

    //软件版本
    InitSoftVersion();

    //获取web版本
    string cat = Command::Execute("cat ../web/static/config/config.js");
    vector<string> catRows = StringEx::Split(cat, "\n", true);
    for (unsigned int i = 0; i < catRows.size(); ++i)
    {
        if (catRows[i].find("window.WebVersionNumbe") != string::npos)
        {
            vector<string> datas = StringEx::Split(catRows[i], " ", true);
            if (datas.size() >= 3 && datas[2].size() >= 3)
            {
                size_t startIndex = datas[2].find_first_of('\'');
                size_t endIndex=datas[2].find_last_of('\'');
                if (startIndex != string::npos
                    && endIndex != string::npos
                    && startIndex != endIndex)
                {
                    _webVersion = datas[2].substr(startIndex+1, endIndex-startIndex-1);
                }
                break;
            }
        }
    }

    _mqtt = new MqttChannel("127.0.0.1", 1883);
    _mqtt->MqttDisconnected.Subscribe(this);
    InitThreads(_mqtt, &_decodes,&_detectors,&_detects, &_recogns);
    if (_sdkInited)
    {
        _mqtt->Start();
        for (unsigned int i = 0; i < _recogns.size(); ++i)
        {
            _recogns[i]->Start();
        }
        for (unsigned int i = 0; i < _detects.size(); ++i)
        {
            _detects[i]->Start();
        }
        while (true)
        {
            bool sdkReady = true;
#ifndef _WIN32
            for (unsigned int i = 0; i < _detects.size(); ++i)
            {
                if (!_detects[i]->Inited())
                {
                    sdkReady = false;
                    break;
                }
            }
            for (unsigned int i = 0; i < _recogns.size(); ++i)
            {
                if (!_recogns[i]->Inited())
                {
                    sdkReady = false;
                    break;
                }
            }
#endif // !_WIN32
            if (sdkReady)
            {
                break;
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
    }
    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        _decodes[i]->Start();
    }
    InitChannels();
    _socketMaid->Start();
    _socketMaid->Join();
    _socketMaid->Stop();
    delete _socketMaid;

    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        _decodes[i]->Stop();
        delete _decodes[i];
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
    if (_mqtt != NULL)
    {
        _mqtt->Stop();
        delete _mqtt;
    }

    SeemmoSDK::Uninit();
    DecodeChannel::UninitHisi(ChannelCount);
    FFmpegChannel::UninitFFmpeg();
    MqttChannel::Uninit();
    Socket::Uninit();
}