#include "TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

const int TrafficStartup::ChannelCount = 8;
const int TrafficStartup::DetectCount = 4;
const int TrafficStartup::RecognCount = 2;

TrafficStartup::TrafficStartup()
    :ThreadObject("startup"), _startTime(DateTime::Now()),_sdkInited(false), _socketMaid(NULL), _mqtt(NULL), _encode(NULL)
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
                JsonSerialization::SerializeValue(&channelJson, "handleSpan",  _decodes[channelIndex - 1]->HandleSpan());
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
    else if (UrlStartWith(e->Url, "/api/gbParameter"))
    {
        TrafficData data;
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            GbParameter parameter=data.GetGbPrameter();
            string json;
            JsonSerialization::SerializeValue(&json, "serverIp", parameter.ServerIp);
            JsonSerialization::SerializeValue(&json, "serverPort", parameter.ServerPort);
            JsonSerialization::SerializeValue(&json, "sipPort", parameter.SipPort);
            JsonSerialization::SerializeValue(&json, "sipType", parameter.SipType);
            JsonSerialization::SerializeValue(&json, "gbId", parameter.GbId);
            JsonSerialization::SerializeValue(&json, "domainId", parameter.DomainId);
            JsonSerialization::SerializeValue(&json, "userName", parameter.UserName);
            JsonSerialization::SerializeValue(&json, "password", parameter.Password);
            e->ResponseJson = json;
            e->Code = HttpCode::OK;
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            JsonDeserialization jd(e->RequestJson);
            GbParameter parameter;
            parameter.ServerIp = jd.Get<string>("serverIp");
            parameter.SipPort = jd.Get<int>("sipPort");
            parameter.SipType = jd.Get<int>("sipType");
            parameter.GbId = jd.Get<string>("gbId");
            parameter.DomainId = jd.Get<string>("domainId");
            parameter.UserName = jd.Get<string>("userName");
            parameter.Password = jd.Get<string>("password");  
            data.SetGbPrameter(parameter);
            e->Code = HttpCode::OK;
        }
    }
    else if (UrlStartWith(e->Url, "/api/gbDevices"))
    {
        TrafficData data;
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            vector<GbDevice> devices = data.GetGbDeviceList();
            for (vector<GbDevice>::iterator it = devices.begin();it!=devices.end();++it)
            {
                string deviceJson;
                JsonSerialization::SerializeValue(&deviceJson, "id", it->Id);
                JsonSerialization::SerializeValue(&deviceJson, "deviceId", it->DeviceId);
                JsonSerialization::SerializeValue(&deviceJson, "deviceName", it->DeviceName);
                JsonSerialization::SerializeValue(&deviceJson, "deviceIp", it->DeviceIp);
                JsonSerialization::SerializeValue(&deviceJson, "devicePort", it->DevicePort);
                JsonSerialization::SerializeValue(&deviceJson, "userName", it->UserName);
                JsonSerialization::SerializeValue(&deviceJson, "password", it->Password);
                JsonSerialization::AddClassItem(&e->ResponseJson, deviceJson);
            }
            e->Code = HttpCode::OK;
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            JsonDeserialization jd(e->RequestJson);
            GbDevice device;
            device.DeviceId = jd.Get<string>("deviceId");
            device.DeviceName = jd.Get<string>("deviceName");
            device.DeviceIp = jd.Get<string>("deviceIp");
            device.DevicePort = jd.Get<int>("devicePort");      
            device.UserName = jd.Get<string>("userName");
            device.Password = jd.Get<string>("password");
            data.InsertGbDevice(device);
            e->Code = HttpCode::OK;
        }
        else if (e->Function.compare(HttpFunction::Put) == 0)
        {
            JsonDeserialization jd(e->RequestJson);
            GbDevice device;
            device.Id = jd.Get<int>("id");
            device.DeviceId = jd.Get<string>("deviceId");
            device.DeviceName = jd.Get<string>("deviceName");
            device.DeviceIp = jd.Get<string>("deviceIp");
            device.DevicePort = jd.Get<int>("devicePort");   
            device.UserName = jd.Get<string>("userName");
            device.Password = jd.Get<string>("password");
            if (data.UpdateGbDevice(device))
            {

                e->Code = HttpCode::OK;
            }
            else
            {
                e->Code = HttpCode::NotFound;
            }
        }
        else if (e->Function.compare(HttpFunction::Delete) == 0)
        {
            string id = GetId(e->Url, "/api/gbDevices");
            if (data.DeleteGbDevice(StringEx::Convert<int>(id)))
            {
                e->Code = HttpCode::OK;
            }
            else
            {
                e->Code = HttpCode::NotFound;
            }
        }
    }
    else if (UrlStartWith(e->Url, "/api/gbChannels"))
    {
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            string gbId = GetId(e->Url, "/api/gbChannels");
            TrafficData data;
            vector<GbChannel> channels=data.GetGbChannelList(gbId);
            for (vector<GbChannel>::iterator it = channels.begin(); it != channels.end(); ++it)
            {
                string channelJson;
                JsonSerialization::SerializeValue(&channelJson, "id", it->Id);
                JsonSerialization::SerializeValue(&channelJson, "channelId", it->ChannelId);
                JsonSerialization::SerializeValue(&channelJson, "channelName", it->ChannelName);
                JsonSerialization::AddClassItem(&e->ResponseJson, channelJson);
            }
            e->Code = HttpCode::OK;
        }
    }
    else if (UrlStartWith(e->Url, "/api/images"))
    {
        string id = GetId(e->Url, "/api/images");
        int channelIndex = StringEx::Convert<int>(id);
        if (ChannelIndexEnable(channelIndex))
        {
            _detectors[channelIndex - 1]->WriteBmp();
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
    else if (UrlStartWith(e->Url, "/api/update/sn"))
    {
        JsonDeserialization jd(e->RequestJson);
        string sn=jd.Get<string>("sn");
        TrafficData data;
        data.SetParameter("SN", sn);
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/logs/export"))
    {
        string ls = Command::Execute("cd ../logs;tar cf logs.tar *.log");
        e->ResponseJson = "logs.tar";
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/report"))
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
    else if (UrlStartWith(e->Url, "/api/test"))
    {
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            if (_encode != NULL)
            {
                _encode->AddOutput(2, StringEx::Combine("../temp/", DateTime::UtcNowTimeStamp(), ".mp4"), 10);
            }
        }
        else  if (e->Function.compare(HttpFunction::Post) == 0)
        {
            string id = GetId(e->Url, "/api/test");
            if (_encode != NULL)
            {
                _encode->OutputFinished(2, StringEx::Combine("../temp/",id));
            }
        }

        e->Code = HttpCode::OK;
    }
}

void TrafficStartup::GetDevice(HttpReceivedEventArgs* e)
{
    TrafficData data;
    string sn = data.GetParameter("SN");
    string guid = StringEx::Trim(Command::Execute("cat /mtd/basesys/data/devguid"));
    string webVersion;
    string cat = Command::Execute("cat ../web/static/config/base.js");
    vector<string> catRows = StringEx::Split(cat, "\n", true);
    for (unsigned int i = 0; i < catRows.size(); ++i)
    {
        if (catRows[i].find("window.WebVersionNumbe") != string::npos)
        {
            vector<string> datas = StringEx::Split(catRows[i], " ", true);
            if (datas.size() >= 3 && datas[2].size() >= 3)
            {
                size_t startIndex = datas[2].find_first_of('\'');
                size_t endIndex = datas[2].find_last_of('\'');
                if (startIndex != string::npos
                    && endIndex != string::npos
                    && startIndex != endIndex)
                {
                    webVersion = datas[2].substr(startIndex + 1, endIndex - startIndex - 1);
                }
                break;
            }
        }
    }
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
            JsonSerialization::SerializeValue(&channelJson, "handleSpan", _decodes[i]->HandleSpan());
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
    JsonSerialization::SerializeValue(&deviceJson, "guid", guid);
    JsonSerialization::SerializeValue(&deviceJson, "softwareVersion", _softwareVersion);
    JsonSerialization::SerializeValue(&deviceJson, "webVersion", webVersion);
    JsonSerialization::SerializeValue(&deviceJson, "sdkVersion", _sdkVersion);
    JsonSerialization::SerializeValue(&deviceJson, "destinationWidth", DecodeChannel::DestinationWidth);
    JsonSerialization::SerializeValue(&deviceJson, "destinationHeight", DecodeChannel::DestinationHeight);
    JsonSerialization::SerializeValue(&deviceJson, "mqttConnected", _mqtt==NULL?0:static_cast<int>(_mqtt->Status()));
    for (unsigned int i = 0; i < _recogns.size(); ++i)
    {
        JsonSerialization::SerializeValue(&deviceJson, StringEx::Combine("recognQueue", i + 1), _recogns[i]->Size());
    }
    JsonSerialization::SerializeClass(&deviceJson, "channels", channelsJson);
    e->Code = HttpCode::OK;
    e->ResponseJson = deviceJson;
}

bool TrafficStartup::ChannelIndexEnable(int channelIndex)
{
    return channelIndex >= 1 && channelIndex <= ChannelCount;
}

string TrafficStartup::CheckChannel(TrafficChannel* channel)
{
    if (ChannelIndexEnable(channel->ChannelIndex))
    {
        if (channel->ChannelType != static_cast<int>(ChannelType::File)
            || channel->Loop)
        {
            channel->Loop = true;
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

void TrafficStartup::FillChannelJson(string* channelJson,const TrafficChannel* channel,const string& host)
{
    JsonSerialization::SerializeValue(channelJson, "channelIndex", channel->ChannelIndex);
    JsonSerialization::SerializeValue(channelJson, "channelName", channel->ChannelName);
    JsonSerialization::SerializeValue(channelJson, "channelUrl", channel->ChannelUrl);
    JsonSerialization::SerializeValue(channelJson, "rtmpUrl", channel->RtmpUrl(host));
    JsonSerialization::SerializeValue(channelJson, "flvUrl", channel->FlvUrl(host));
    JsonSerialization::SerializeValue(channelJson, "channelType", channel->ChannelType);
    JsonSerialization::SerializeValue(channelJson, "deviceId", channel->DeviceId);
    JsonSerialization::SerializeValue(channelJson, "loop", channel->Loop);
    JsonSerialization::SerializeValue(channelJson, "outputReport", channel->OutputReport);
    JsonSerialization::SerializeValue(channelJson, "outputImage", channel->OutputImage);
    JsonSerialization::SerializeValue(channelJson, "outputRecogn", channel->OutputRecogn);
    JsonSerialization::SerializeValue(channelJson, "globalDetect", channel->GlobalDetect);
}

void TrafficStartup::FillChannel(TrafficChannel* channel,const JsonDeserialization& jd)
{
    channel->ChannelIndex = jd.Get<int>("channelIndex");
    channel->ChannelName = jd.Get<string>("channelName");
    channel->ChannelUrl = jd.Get<string>("channelUrl");
    channel->ChannelType = jd.Get<int>("channelType");
    channel->DeviceId = jd.Get<string>("deviceId");
    channel->Loop = jd.Get<bool>("loop");
    channel->OutputImage = jd.Get<bool>("outputImage");
    channel->OutputReport = jd.Get<bool>("outputReport");
    channel->OutputRecogn = jd.Get<bool>("outputRecogn");
    channel->GlobalDetect = jd.Get<bool>("globalDetect");
}

void TrafficStartup::FillChannel(TrafficChannel* channel, const JsonDeserialization& jd, int itemIndex)
{
    channel->ChannelIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":channelIndex"));
    channel->ChannelName = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":channelName"));
    channel->ChannelUrl = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":channelUrl"));
    channel->ChannelType = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":channelType"));
    channel->DeviceId = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":deviceId"));
}

void TrafficStartup::StartCore()
{
    Socket::Init();
    MqttChannel::Init();
    FFmpegInput::InitFFmpeg();

    DecodeChannel::UninitHisi(ChannelCount);
    if (!DecodeChannel::InitHisi(ChannelCount)
        ||!EncodeChannel::InitHisi(ChannelCount, DecodeChannel::DestinationWidth,DecodeChannel::DestinationHeight))
    {
        exit(2);
    }

    _socketMaid = new SocketMaid(2,false);
    _handler.HttpReceived.Subscribe(this);
    if (_socketMaid->AddListenEndPoint(EndPoint(7772), &_handler) == -1)
    {
        exit(2);
    }

    Path::CreatePath("../temp");
    Path::CreatePath("../images");
    Command::Execute("rm -rf ../temp/*");
    Command::Execute("rm -rf ../images/*");

    //升级数据库
    UpdateDb();

    //软件版本
    TrafficData data;
    _softwareVersion = data.GetParameter("Version");

    //初始化sdk
    _sdkInited = SeemmoSDK::Init();
    if (SeemmoSDK::seemmo_version != NULL)
    {
        _sdkVersion = SeemmoSDK::seemmo_version();
    }
    int loginHandler = -1;
#ifndef _WIN32
    int gbResult = vas_sdk_startup();
    if (gbResult >= 0)
    {
        LogPool::Information(LogEvent::System, "国标sdk初始化成功，登陆句柄:",gbResult);
    }
    else
    {
        LogPool::Information(LogEvent::System, "国标sdk初始化失败,错误代码:", gbResult);
    }

    GbParameter gbParameter = data.GetGbPrameter();
    if (gbParameter.ServerIp.empty())
    {
        LogPool::Information(LogEvent::System, "国标服务地址未配置，略过登陆");
    }
    else
    {
        loginHandler = vas_sdk_login(const_cast<char*>(gbParameter.ServerIp.c_str()), gbParameter.ServerPort, const_cast<char*>(gbParameter.UserName.c_str()), const_cast<char*>(gbParameter.Password.c_str()));
        if (loginHandler >= 0)
        {
            LogPool::Information(LogEvent::System, "国标sdk登陆成功");
        }
        else
        {
            LogPool::Information(LogEvent::System, "国标sdk登陆失败,错误代码:", loginHandler);
        }
    }
#endif // !_WIN32

    _mqtt = new MqttChannel("127.0.0.1", 1883);
    _mqtt->MqttDisconnected.Subscribe(this);
    _encode = new EncodeChannel(ChannelCount);
    InitThreads(_mqtt, &_decodes,_encode,&_detectors,&_detects, &_recogns, loginHandler);
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
    _encode->Start();
    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        _decodes[i]->Start();
    }
    InitChannels();
    _socketMaid->Start();
  
    while (!_cancelled)
    {
        string ps = Command::Execute("top -n 1|grep Genos.out");
        vector<string> psRows = StringEx::Split(ps, "\n", true);
        if (!psRows.empty())
        {
            vector<string> columns = StringEx::Split(psRows[0], " ", true);
            if (columns.size() >= 5)
            {
                vector<string> datas = StringEx::Split(columns[4], "m", true);
                if (!datas.empty())
                {
                    LogPool::Information(LogEvent::Monitor, "memory:", datas[0]);
                }
            }
        }
        this_thread::sleep_for(chrono::minutes(1));
    }

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
    EncodeChannel::UninitHisi(ChannelCount);
    DecodeChannel::UninitHisi(ChannelCount);
    FFmpegInput::UninitFFmpeg();
    MqttChannel::Uninit();
    Socket::Uninit();
}