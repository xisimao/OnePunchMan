#include "TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

const int TrafficStartup::ChannelCount = 8;
const int TrafficStartup::DetectCount = 4;
const int TrafficStartup::RecognCount = 2;

TrafficStartup::TrafficStartup()
    :_startTime(DateTime::Now()),_sdkInited(false), _socketMaid(NULL)
    , _mqtt(NULL), _data(NULL),_encode(NULL)
{
    JsonDeserialization jd("appsettings.json");
    FlowDetector::Init(jd);
    EventDetector::Init(jd);
}

TrafficStartup::~TrafficStartup()
{
    for (unsigned int i = 0; i < _flowDetectors.size(); ++i)
    {
        delete _flowDetectors[i];
    }
    for (unsigned int i = 0; i < _eventDetectors.size(); ++i)
    {
        delete _eventDetectors[i];
    }
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
    else if (UrlStartWith(e->Url, "/api/gbparameter"))
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
    else if (UrlStartWith(e->Url, "/api/gbdevices"))
    {
        TrafficData data;
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            vector<GbDevice> devices = data.GetGbDevices();
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
            string id = GetId(e->Url, "/api/gbdevices");
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
    else if (UrlStartWith(e->Url, "/api/gbchannels"))
    {
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            string gbId = GetId(e->Url, "/api/gbChannels");
            TrafficData data;
            vector<GbChannel> channels=data.GetGbChannels(gbId);
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
            _detects[channelIndex]->WriteBmp(channelIndex);
            e->Code = HttpCode::OK;
        }
        else
        {
            e->Code = HttpCode::NotFound;
        }
    }
    else if (UrlStartWith(e->Url, "/api/update/licence"))
    {
        LogPool::Information(LogEvent::System, "更新授权文件");
        string filePath("/mtd/seemmo/programs/aisdk/data/licence");
        HttpHandler::WriteFile(e->RequestJson, filePath);
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/update/system"))
    {
        LogPool::Information(LogEvent::System, "更新系统");
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
    //else if (UrlStartWith(e->Url, "/api/logs"))
    //{
    //    vector<string> datas = StringEx::Split(e->Url, "?", true);
    //    if (datas.size() > 1)
    //    {
    //        int logLevel = 0;
    //        int logEvent = 0;
    //        string startTime;
    //        string endTime;
    //        int pageNum = 0;
    //        int pageSize = 0;
    //        vector<string> params = StringEx::Split(datas[1], "&", true);
    //        for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
    //        {
    //            vector<string> pair = StringEx::Split(*mit, "=", true);
    //            if (pair.size() >= 2)
    //            {
    //                if (pair[0].compare("logLevel") == 0)
    //                {
    //                    logLevel = StringEx::Convert<int>(pair[1]);
    //                }
    //                else if (pair[0].compare("logEvent") == 0)
    //                {
    //                    logEvent = StringEx::Convert<int>(pair[1]);
    //                }
    //                else if (pair[0].compare("startTime") == 0)
    //                {
    //                    startTime = pair[1];
    //                    if (startTime.size() >= 19)
    //                    {
    //                        startTime = StringEx::Replace(startTime, "%20", " ");
    //                        startTime = StringEx::Replace(startTime, "%3A", ":");
    //                    }
    //                }
    //                else if (pair[0].compare("endTime") == 0)
    //                {
    //                    endTime = pair[1];
    //                    if (endTime.size() >= 19)
    //                    {
    //                        endTime = StringEx::Replace(endTime, "%20", " ");
    //                        endTime = StringEx::Replace(endTime, "%3A", ":");
    //                    }
    //                }
    //                else if (pair[0].compare("pageNum") == 0)
    //                {
    //                    pageNum = StringEx::Convert<int>(pair[1]);
    //                }
    //                else if (pair[0].compare("pageSize") == 0)
    //                {
    //                    pageSize = StringEx::Convert<int>(pair[1]);
    //                }
    //            }
    //        }
    //        int total = 0;
    //        vector<LogData> logDatas = SqliteLogger::GetDatas(logLevel, logEvent, startTime, endTime, pageNum, pageSize,&total);
    //        string datasJson;
    //        for (vector<LogData>::iterator it = logDatas.begin(); it != logDatas.end(); ++it)
    //        {
    //            string json;
    //            JsonSerialization::SerializeValue(&json, "id", it->Id);
    //            JsonSerialization::SerializeValue(&json, "logLevel", it->LogLevel);
    //            JsonSerialization::SerializeValue(&json, "logEvent", it->LogEvent);
    //            JsonSerialization::SerializeValue(&json, "time", it->Time);
    //            JsonSerialization::SerializeValue(&json, "content", it->Content);
    //            JsonSerialization::AddClassItem(&datasJson, json);
    //        }
    //        JsonSerialization::SerializeValue(&e->ResponseJson, "total", total);
    //        JsonSerialization::SerializeArray(&e->ResponseJson, "datas", datasJson);
    //    }
    //    e->Code = HttpCode::OK;
    //}
    else if (UrlStartWith(e->Url, "/api/report"))
    {
        string id = GetId(e->Url, "/api/report");
        int channelIndex = StringEx::Convert<int>(id);
        if (ChannelIndexEnable(channelIndex))
        {
            _flowDetectors[channelIndex - 1]->GetReportJson(&e->ResponseJson);
            e->Code = HttpCode::OK;
        }
        else
        {
            e->Code = HttpCode::NotFound;
        }
    }
    else if (UrlStartWith(e->Url, "/api/data"))
    {
        int channelIndex = StringEx::Convert<int>(GetId(e->Url, "/api/data"));
        TrafficData data;
        vector<EventData> datas = data.GetEventDatas(channelIndex,0,0);
        for (vector<EventData>::iterator it = datas.begin(); it != datas.end(); ++it)
        {
            string json;
            JsonSerialization::SerializeValue(&json, "id", it->Id);
            string guid = it->Guid;
            JsonSerialization::SerializeValue(&json, "channelIndex", it->ChannelIndex);
            JsonSerialization::SerializeValue(&json, "laneIndex", it->LaneIndex);
            JsonSerialization::SerializeValue(&json, "timeStamp", it->TimeStamp);
            JsonSerialization::SerializeValue(&json, "type", it->Type);
            JsonSerialization::SerializeValue(&json, "image1", TrafficDirectory::GetImageLink(guid, 1));
            JsonSerialization::SerializeValue(&json, "image2", TrafficDirectory::GetImageLink(guid, 2));
            JsonSerialization::SerializeValue(&json, "video", TrafficDirectory::GetVideoLink(guid));
            JsonSerialization::AddClassItem(&e->ResponseJson, json);
        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/account/login"))
    {
        JsonDeserialization jd(e->RequestJson);
        string userName=jd.Get<string>("userName");
        string password=jd.Get<string>("password");
        if (userName.compare("admin") == 0 && password.compare("admin") == 0)
        {
            e->Code = HttpCode::OK;
        }
        else
        {
            e->Code = HttpCode::BadRequest;
        }
    }
    else if (UrlStartWith(e->Url, "/api/system/time"))
    {
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            JsonSerialization::SerializeValue(&e->ResponseJson,"dateTime", DateTime::Now().ToString());
            e->Code = HttpCode::OK;
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            JsonDeserialization jd(e->RequestJson);
            string value = jd.Get<string>("dateTime");
            DateTime dateTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", value);
            Command::Execute(StringEx::Combine("date -s ", dateTime.ToString("%Y%m%d%H%M.%S")));
            e->Code = HttpCode::OK;
        }     
    }
#ifndef _WIN32
    else if (UrlStartWith(e->Url, "/api/system/ip"))
    {
    if (e->Function.compare(HttpFunction::Get) == 0)
    {
        string id = GetId(e->Url, "/api/system/ip");
        JsonSerialization::SerializeValue(&e->ResponseJson, "ip", EndPoint::GetIp(id));
        JsonSerialization::SerializeValue(&e->ResponseJson, "mask", EndPoint::GetMask(id));
        JsonSerialization::SerializeValue(&e->ResponseJson, "gateway", EndPoint::GetGateway(id));
        JsonSerialization::SerializeValue(&e->ResponseJson, "mac", EndPoint::GetMac(id));
        e->Code = HttpCode::OK;
    }
    else if (e->Function.compare(HttpFunction::Post) == 0)
    {
        JsonDeserialization jd(e->RequestJson);
        string deviceName = jd.Get<string>("deviceName");
        string ip = jd.Get<string>("ip");
        string mask = jd.Get<string>("mask");
        string gateway = jd.Get<string>("gateway");

        string oldIp = EndPoint::GetIp(deviceName);
        string oldMask = EndPoint::GetMask(deviceName);
        string oldGateway = EndPoint::GetGateway(deviceName);
        if (EndPoint::SetIp(deviceName, ip)
            && EndPoint::SetMask(deviceName, mask)
            && EndPoint::SetGateway(deviceName, gateway))
        {
            e->Code = HttpCode::OK;
        }
        else
        {
            EndPoint::SetIp(deviceName, oldIp);
            EndPoint::SetMask(deviceName, oldMask);
            EndPoint::SetGateway(deviceName, oldGateway);
            e->Code = HttpCode::BadRequest;
        }
    }
    }
#endif // !_WIN32
    else if (UrlStartWith(e->Url, "/api/system/reboot"))
    {
        Command::Execute("reboot");
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/system/export"))
    {
        Command::Execute("tar cf logs.tar logs flow.db");
        JsonSerialization::SerializeValue(&e->ResponseJson, "url", "bin/logs.tar");
        e->Code = HttpCode::OK;
    }
}

void TrafficStartup::GetDevice(HttpReceivedEventArgs* e)
{
    TrafficData data;
    string sn = data.GetParameter("SN");
    string guid = StringEx::Trim(Command::Execute("cat /mtd/basesys/data/devguid"));
    string webVersion;
    string cat = Command::Execute(StringEx::Combine("cat ",TrafficDirectory::WebConfigPath));
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
            JsonSerialization::AddClassItem(&channelsJson, channelJson);
        }
    }

    string deviceJson;
    DateTime now = DateTime::Now();
    DateTime adjustTime = DateTime(now.Year() + 1, now.Month(), now.Day(), now.Hour(), now.Minute(), now.Second(), now.Millisecond());
    JsonSerialization::SerializeValue(&deviceJson, "deviceTime", adjustTime.TimeStamp());
    JsonSerialization::SerializeValue(&deviceJson, "deviceTime_Desc", adjustTime.ToString());
    JsonSerialization::SerializeValue(&deviceJson, "startTime", _startTime.ToString());
    JsonSerialization::SerializeValue(&deviceJson, "diskUsed", diskUsed);
    JsonSerialization::SerializeValue(&deviceJson, "diskTotal", diskTotal);
    JsonSerialization::SerializeValue(&deviceJson, "licenceStatus", _sdkInited);
    JsonSerialization::SerializeValue(&deviceJson, "sn", sn);
    JsonSerialization::SerializeValue(&deviceJson, "guid", guid);
    JsonSerialization::SerializeValue(&deviceJson, "softwareVersion", _softwareVersion);
    JsonSerialization::SerializeValue(&deviceJson, "webVersion", webVersion);
    JsonSerialization::SerializeValue(&deviceJson, "sdkVersion", _sdkVersion);
    JsonSerialization::SerializeValue(&deviceJson, "mqttConnected", _mqtt==NULL?0:static_cast<int>(_mqtt->Status()));
    for (unsigned int i = 0; i < _recogns.size(); ++i)
    {
        JsonSerialization::SerializeValue(&deviceJson, StringEx::Combine("recognQueue", i + 1), _recogns[i]->Size());
    }
    JsonSerialization::SerializeClass(&deviceJson, "channels", channelsJson);
    e->Code = HttpCode::OK;
    e->ResponseJson = deviceJson;
}

void TrafficStartup::SetDevice(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    int itemIndex = 0;
    vector<TrafficChannel> channels;
    map<int, TrafficChannel> tempChannels;
    while (true)
    {
        TrafficChannel channel;
        FillChannel(&channel, jd, itemIndex);
        if (channel.ChannelIndex == 0)
        {
            break;
        }
        int flowLaneIndex = 0;
        while (true)
        {
            FlowLane lane;
            lane.LaneId = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":laneId"));
            if (lane.LaneId.empty())
            {
                break;
            }
            lane.ChannelIndex = channel.ChannelIndex;
            lane.LaneName = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":laneName"));
            lane.LaneIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":laneIndex"));
            lane.Direction = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":direction"));
            lane.FlowDirection = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":flowDirection"));
            lane.IOIp = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":ioIp"));
            lane.IOPort = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":ioPort"));
            lane.IOIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":ioIndex"));
            lane.StopLine = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":stopLine"));
            lane.FlowDetectLine = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":flowDetectLine"));
            lane.QueueDetectLine = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":queueDetectLine"));
            lane.LaneLine1 = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":laneLine1"));
            lane.LaneLine2 = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":flowLans:", flowLaneIndex, ":laneLine2"));
            channel.FlowLanes.push_back(lane);
            flowLaneIndex += 1;
        }

        int eventLaneIndex = 0;
        while (true)
        {
            EventLane lane;
            lane.LaneIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":eventLanes:", eventLaneIndex, ":laneIndex"));
            if (lane.LaneIndex == 0)
            {
                break;
            }
            lane.ChannelIndex = channel.ChannelIndex;
            lane.LaneName = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":eventLanes:", eventLaneIndex, ":laneName"));
            lane.LaneType = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":eventLanes:", eventLaneIndex, ":laneType"));
            lane.Region = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":eventLanes:", eventLaneIndex, ":region"));
            lane.Line = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":eventLanes:", eventLaneIndex, ":line"));
            channel.EventLanes.push_back(lane);
            eventLaneIndex += 1;
        }

        string error = CheckChannel(&channel);
        if (!error.empty())
        {
            e->Code = HttpCode::BadRequest;
            e->ResponseJson = error;
            return;
        }
        channels.push_back(channel);
        tempChannels.insert(pair<int, TrafficChannel>(channel.ChannelIndex, channel));
        itemIndex += 1;
    }
    TrafficData data;
    if (data.SetChannels(channels))
    {
        for (int i = 0; i < ChannelCount; ++i)
        {
            map<int, TrafficChannel>::iterator it = tempChannels.find(i + 1);
            if (it == tempChannels.end())
            {
                _decodes[i]->ClearChannel();
                _flowDetectors[i]->ClearChannel();
                _eventDetectors[i]->ClearChannel();
            }
            else
            {
                unsigned char taskId = _decodes[i]->UpdateChannel(it->second);
                _detects[i + 1]->UpdateChannel(it->second);
                _flowDetectors[i]->UpdateChannel(taskId, it->second);
                _eventDetectors[i]->UpdateChannel(taskId, it->second);
            }
        }
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::BadRequest;
        JsonSerialization::SerializeValue(&e->ResponseJson, "message", data.LastError());
    }
}

void TrafficStartup::SetChannel(HttpReceivedEventArgs* e)
{
    JsonDeserialization jd(e->RequestJson);
    TrafficChannel channel;
    FillChannel(&channel, jd);
    int flowLaneIndex = 0;
    while (true)
    {
        FlowLane lane;
        lane.LaneId = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneId"));
        if (lane.LaneId.empty())
        {
            break;
        }
        lane.ChannelIndex = channel.ChannelIndex;
        lane.LaneName = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneName"));
        lane.LaneIndex = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneIndex"));
        lane.Direction = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":direction"));
        lane.FlowDirection = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":flowDirection"));
        lane.StopLine = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":stopLine"));
        lane.FlowDetectLine = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":flowDetectLine"));
        lane.QueueDetectLine = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":queueDetectLine"));
        lane.LaneLine1 = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneLine1"));
        lane.LaneLine2 = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneLine2"));
        lane.IOIp = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":ioIp"));
        lane.IOPort = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":ioPort"));
        lane.IOIndex = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":ioIndex"));
        channel.FlowLanes.push_back(lane);
        flowLaneIndex += 1;
    }

    int eventLaneIndex = 0;
    while (true)
    {
        EventLane lane;
        lane.LaneIndex = jd.Get<int>(StringEx::Combine("eventLanes:", eventLaneIndex, ":laneIndex"));
        if (lane.LaneIndex == 0)
        {
            break;
        }
        lane.ChannelIndex = channel.ChannelIndex;
        lane.LaneName = jd.Get<string>(StringEx::Combine("eventLanes:", eventLaneIndex, ":laneName"));
        lane.LaneType = jd.Get<int>(StringEx::Combine("eventLanes:", eventLaneIndex, ":laneType"));
        lane.Region = jd.Get<string>(StringEx::Combine("eventLanes:", eventLaneIndex, ":region"));
        lane.Line = jd.Get<string>(StringEx::Combine("eventLanes:", eventLaneIndex, ":line"));
        channel.EventLanes.push_back(lane);
        eventLaneIndex += 1;
    }

    string error = CheckChannel(&channel);
    if (!error.empty())
    {
        e->Code = HttpCode::BadRequest;
        e->ResponseJson = error;
        return;
    }
    TrafficData data;
    if (data.SetChannel(channel))
    {
        unsigned char taskId = _decodes[channel.ChannelIndex - 1]->UpdateChannel(channel);
        _detects[channel.ChannelIndex]->UpdateChannel(channel);
        _flowDetectors[channel.ChannelIndex - 1]->UpdateChannel(taskId, channel);
        _eventDetectors[channel.ChannelIndex - 1]->UpdateChannel(taskId, channel);
        e->Code = HttpCode::OK;
    }
    else
    {
        e->Code = HttpCode::BadRequest;
        JsonSerialization::SerializeValue(&e->ResponseJson, "message", data.LastError());
    }
}

bool TrafficStartup::DeleteChannel(int channelIndex)
{
    TrafficData data;
    if (data.DeleteChannel(channelIndex))
    {
        _decodes[channelIndex - 1]->ClearChannel();
        _flowDetectors[channelIndex - 1]->ClearChannel();
        _eventDetectors[channelIndex - 1]->ClearChannel();
        return true;
    }
    else
    {
        return false;
    }
}

bool TrafficStartup::ChannelIndexEnable(int channelIndex)
{
    return channelIndex >= 1 && channelIndex <= ChannelCount;
}

string TrafficStartup::CheckChannel(TrafficChannel* channel)
{
    if (!ChannelIndexEnable(channel->ChannelIndex))
    {
        string json;
        JsonSerialization::SerializeValue(&json, "message", WStringEx::Combine(L"\u901a\u9053\u5e8f\u53f7\u5fc5\u987b\u5728 1-", ChannelCount, L" \u4e4b\u95f4"));
        return json;
    }
  
    TrafficData data;
    for (vector<FlowLane>::iterator it = channel->FlowLanes.begin(); it != channel->FlowLanes.end(); ++it)
    {
        //构建检测区域
        BrokenLine laneLine1 = BrokenLine::FromJson(it->LaneLine1);
        BrokenLine laneLine2 = BrokenLine::FromJson(it->LaneLine2);
        Line stopLine = Line::FromJson(it->StopLine);
        Line flowDetectLine = Line::FromJson(it->FlowDetectLine);
        Line queueDetectLine = Line::FromJson(it->QueueDetectLine);
        Polygon flowRegion = Polygon::Build(laneLine1, laneLine2, stopLine, flowDetectLine);
        if (!flowDetectLine.Empty()&&flowRegion.Empty())
        {
            string json;
            JsonSerialization::SerializeValue(&json, "message", WStringEx::ToString(L"\u6d41\u91cf\u68c0\u6d4b\u533a\u57df\u6ca1\u6709\u95ed\u5408"));
            return json;
        }
        Polygon queueRegion = Polygon::Build(laneLine1, laneLine2, stopLine, queueDetectLine);
        if (!queueDetectLine.Empty()&&queueRegion.Empty())
        {
            string json;
            JsonSerialization::SerializeValue(&json, "message", WStringEx::ToString(L"\u6392\u961f\u68c0\u6d4b\u533a\u57df\u6ca1\u6709\u95ed\u5408"));
            return json;
        }
        it->FlowRegion = flowRegion.ToJson();
        it->QueueRegion = queueRegion.ToJson();
        vector<FlowLane> lanes = data.GetFlowLanes(channel->ChannelIndex, it->LaneId);
        for (vector<FlowLane>::iterator lit = lanes.begin(); lit != lanes.end(); ++lit)
        {
            if (channel->ReportProperties & lit->ReportProperties)
            {
                string json;
                JsonSerialization::SerializeValue(&json, "message", WStringEx::Combine(L"\u7b2c", it->ChannelIndex, L"\u4e2a\u901a\u9053\u7684\u7b2c", it->LaneIndex, L"\u4e2a\u8f66\u9053\u914d\u7f6e\u7684\u68c0\u6d4b\u9879\u4e0e\u7b2c", lit->ChannelIndex, L"\u4e2a\u901a\u9053\u7684\u7b2c", lit->LaneIndex,L"\u4e2a\u8f66\u9053\u91cd\u590d"));
                return json;
            }
        }
    }

    //调整文件模式下的设置
    if (channel->ChannelType != static_cast<int>(ChannelType::File)
        || channel->Loop)
    {
        channel->Loop = true;
        channel->OutputDetect = false;
        channel->OutputImage = false;
        channel->OutputReport = false;
        channel->OutputRecogn = false;
        channel->GlobalDetect = false;
    }
    return string();
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
            && (url[key.size()] == '/'|| url[key.size()] == '?');
    }
    else
    {
        return false;
    }
}

string TrafficStartup::GetId(const std::string& url, const std::string& key)
{
    return url.size() > key.size() ? url.substr(key.size() + 1, url.size() - key.size() - 1) : string();
}

void TrafficStartup::FillChannelJson(string* channelJson,const TrafficChannel* channel,const string& host)
{
    JsonSerialization::SerializeValue(channelJson, "channelIndex", channel->ChannelIndex);
    JsonSerialization::SerializeValue(channelJson, "channelName", channel->ChannelName);
    JsonSerialization::SerializeValue(channelJson, "channelUrl", channel->ChannelUrl);
    JsonSerialization::SerializeValue(channelJson, "rtmpUrl", channel->RtmpUrl(host));
    JsonSerialization::SerializeValue(channelJson, "flvUrl", channel->FlvUrl(host));
    JsonSerialization::SerializeValue(channelJson, "channelType", channel->ChannelType);
    JsonSerialization::SerializeValue(channelJson, "channelStatus", static_cast<int>(_decodes[channel->ChannelIndex-1]->Status()));
    JsonSerialization::SerializeValue(channelJson, "frameSpan", static_cast<int>(_decodes[channel->ChannelIndex-1]->FrameSpan()));
    JsonSerialization::SerializeValue(channelJson, "sourceWidth", static_cast<int>(_decodes[channel->ChannelIndex-1]->SourceWidth()));
    JsonSerialization::SerializeValue(channelJson, "sourceHeight", static_cast<int>(_decodes[channel->ChannelIndex-1]->SourceHeight()));
    JsonSerialization::SerializeValue(channelJson, "destinationWidth", DecodeChannel::DestinationWidth);
    JsonSerialization::SerializeValue(channelJson, "destinationHeight", DecodeChannel::DestinationHeight);
    JsonSerialization::SerializeValue(channelJson, "deviceId", channel->DeviceId);
    JsonSerialization::SerializeValue(channelJson, "loop", channel->Loop);
    JsonSerialization::SerializeValue(channelJson, "outputDetect", channel->OutputDetect);
    JsonSerialization::SerializeValue(channelJson, "outputImage", channel->OutputImage);
    JsonSerialization::SerializeValue(channelJson, "outputReport", channel->OutputReport);
    JsonSerialization::SerializeValue(channelJson, "outputRecogn", channel->OutputRecogn);
    JsonSerialization::SerializeValue(channelJson, "globalDetect", channel->GlobalDetect);
    JsonSerialization::SerializeValue(channelJson, "laneWidth", channel->LaneWidth);
    JsonSerialization::SerializeValue(channelJson, "reportProperties", channel->ReportProperties);
    JsonSerialization::SerializeValue(channelJson, "freeSpeed", channel->FreeSpeed);

}

void TrafficStartup::FillChannel(TrafficChannel* channel,const JsonDeserialization& jd)
{
    channel->ChannelIndex = jd.Get<int>("channelIndex");
    channel->ChannelName = jd.Get<string>("channelName");
    channel->ChannelUrl = jd.Get<string>("channelUrl");
    channel->ChannelType = jd.Get<int>("channelType");
    channel->DeviceId = jd.Get<string>("deviceId");

    channel->Loop = jd.Get<bool>("loop");
    channel->OutputDetect = jd.Get<bool>("outputDetect");
    channel->OutputImage = jd.Get<bool>("outputImage");
    channel->OutputReport = jd.Get<bool>("outputReport");
    channel->OutputRecogn = jd.Get<bool>("outputRecogn");
    channel->GlobalDetect = jd.Get<bool>("globalDetect");

    channel->LaneWidth = jd.Get<double>("laneWidth");
    channel->ReportProperties = jd.Get<int>("reportProperties");
    channel->FreeSpeed = jd.Get<double>("freeSpeed");
}

void TrafficStartup::FillChannel(TrafficChannel* channel, const JsonDeserialization& jd, int itemIndex)
{
    channel->ChannelIndex = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":channelIndex"));
    channel->ChannelName = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":channelName"));
    channel->ChannelUrl = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":channelUrl"));
    channel->ChannelType = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":channelType"));
    channel->DeviceId = jd.Get<string>(StringEx::Combine("channels:", itemIndex, ":deviceId"));
    
    channel->Loop = jd.Get<bool>(StringEx::Combine("channels:", itemIndex, ":loop"));
    channel->OutputDetect = jd.Get<bool>(StringEx::Combine("channels:", itemIndex, ":outputDetect"));
    channel->OutputImage = jd.Get<bool>(StringEx::Combine("channels:", itemIndex, ":outputImage"));
    channel->OutputReport = jd.Get<bool>(StringEx::Combine("channels:", itemIndex, ":outputReport"));
    channel->OutputRecogn = jd.Get<bool>(StringEx::Combine("channels:", itemIndex, ":outputRecogn"));
    channel->GlobalDetect = jd.Get<bool>(StringEx::Combine("channels:", itemIndex, ":globalDetect"));

    channel->LaneWidth = jd.Get<double>(StringEx::Combine("channels:", itemIndex, ":laneWidth"));
    channel->ReportProperties = jd.Get<int>(StringEx::Combine("channels:", itemIndex, ":reportProperties"));
    channel->FreeSpeed = jd.Get<double>(StringEx::Combine("channels:", itemIndex, ":freeSpeed"));
}

string TrafficStartup::GetChannelJson(const string& host, int channelIndex)
{
    string channelJson;
    TrafficData data;
    TrafficChannel channel = data.GetChannel(channelIndex);
    if (!channel.ChannelUrl.empty())
    {
        FillChannelJson(&channelJson, &channel, host);
        if (ChannelIndexEnable(channel.ChannelIndex))
        {
            JsonSerialization::SerializeValue(&channelJson, "lanesInited", _flowDetectors[channel.ChannelIndex - 1]->LanesInited());
        }

        string flowLanesJson;
        for (vector<FlowLane>::const_iterator lit = channel.FlowLanes.begin(); lit != channel.FlowLanes.end(); ++lit)
        {
            string laneJson;
            JsonSerialization::SerializeValue(&laneJson, "channelIndex", lit->ChannelIndex);
            JsonSerialization::SerializeValue(&laneJson, "laneId", lit->LaneId);
            JsonSerialization::SerializeValue(&laneJson, "laneName", lit->LaneName);
            JsonSerialization::SerializeValue(&laneJson, "laneIndex", lit->LaneIndex);
            JsonSerialization::SerializeValue(&laneJson, "direction", lit->Direction);
            JsonSerialization::SerializeValue(&laneJson, "flowDirection", lit->FlowDirection);
            JsonSerialization::SerializeValue(&laneJson, "ioIp", lit->IOIp);
            JsonSerialization::SerializeValue(&laneJson, "ioPort", lit->IOPort);
            JsonSerialization::SerializeValue(&laneJson, "ioIndex", lit->IOIndex);
            JsonSerialization::SerializeValue(&laneJson, "stopLine", lit->StopLine);
            JsonSerialization::SerializeValue(&laneJson, "flowDetectLine", lit->FlowDetectLine);
            JsonSerialization::SerializeValue(&laneJson, "queueDetectLine", lit->QueueDetectLine);
            JsonSerialization::SerializeValue(&laneJson, "laneLine1", lit->LaneLine1);
            JsonSerialization::SerializeValue(&laneJson, "laneLine2", lit->LaneLine2);
            JsonSerialization::SerializeValue(&laneJson, "flowRegion", lit->FlowRegion);
            JsonSerialization::SerializeValue(&laneJson, "queueRegion", lit->QueueRegion);
            JsonSerialization::AddClassItem(&flowLanesJson, laneJson);
        }
        JsonSerialization::SerializeArray(&channelJson, "flowLanes", flowLanesJson);

        string eventLanesJson;
        for (vector<EventLane>::const_iterator lit = channel.EventLanes.begin(); lit != channel.EventLanes.end(); ++lit)
        {
            string laneJson;
            JsonSerialization::SerializeValue(&laneJson, "channelIndex", lit->ChannelIndex);
            JsonSerialization::SerializeValue(&laneJson, "laneIndex", lit->LaneIndex);
            JsonSerialization::SerializeValue(&laneJson, "laneName", lit->LaneName);
            JsonSerialization::SerializeValue(&laneJson, "laneType", lit->LaneType);
            JsonSerialization::SerializeValue(&laneJson, "region", lit->Region);
            JsonSerialization::SerializeValue(&laneJson, "line", lit->Line);
            JsonSerialization::AddClassItem(&eventLanesJson, laneJson);
        }
        JsonSerialization::SerializeArray(&channelJson, "eventLanes", eventLanesJson);
    }
    return channelJson;
}

void TrafficStartup::InitThreads(int loginHandler)
{
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowDetector* flowDetector = new FlowDetector(DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight,_socketMaid, _mqtt, _data);
        _flowDetectors.push_back(flowDetector);
        EventDetector* eventDetector = new EventDetector(DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, _mqtt, _encode, _data);
        _eventDetectors.push_back(eventDetector);
        DecodeChannel* decode = new DecodeChannel(i + 1, loginHandler, _encode);
        _decodes.push_back(decode);
    }

    for (int i = 0; i < RecognCount; ++i)
    {
        RecognChannel* recogn = new RecognChannel(i, DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight, &_flowDetectors);
        _recogns.push_back(recogn);
    }
    vector<DetectChannel*> detects;
    for (int i = 0; i < DetectCount; ++i)
    {
        DetectChannel* detect = new DetectChannel(i, DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight);
        detects.push_back(detect);
    }

    for (int i = 0; i < DetectCount; ++i)
    {
        detects.at(i)->SetRecogn(_recogns.at(i % RecognCount));
        for (int j = 0; j < ChannelCount / DetectCount; ++j)
        {
            int channelIndex = i + (j * DetectCount) + 1;
            detects.at(i)->AddChannel(channelIndex, _decodes.at(channelIndex - 1), _flowDetectors.at(channelIndex - 1), _eventDetectors.at(channelIndex - 1));
            _detects.insert(pair<int, DetectChannel*>(channelIndex, detects.at(i)));
        }
    }
}

void TrafficStartup::InitChannels()
{
    TrafficData data;
    set<EndPoint> ips;
    for (int i = 0; i < ChannelCount; ++i)
    {
        TrafficChannel channel = data.GetChannel(i + 1);
        if (!channel.ChannelUrl.empty())
        {
            //io转换器地址
            for (vector<FlowLane>::iterator lit = channel.FlowLanes.begin(); lit != channel.FlowLanes.end(); ++lit)
            {
                if (!lit->IOIp.empty())
                {
                    ips.insert(EndPoint(lit->IOIp, 24000));
                }
            }
            unsigned char taskId = _decodes[i]->UpdateChannel(channel);
            _detects[i+1]->UpdateChannel(channel);
            _flowDetectors[i]->UpdateChannel(taskId, channel);
            _eventDetectors[i]->UpdateChannel(taskId, channel);
        }
    }
    for (set<EndPoint>::iterator it = ips.begin(); it != ips.end(); ++it)
    {
        LogPool::Information(LogEvent::System, "连接到io转换器", it->HostIp());
        _socketMaid->AddConnectEndPoint(*it, NULL);
    }
}

void TrafficStartup::Start()
{
    Socket::Init();
    MqttChannel::Init();
    DecodeChannel::InitFFmpeg();

    DecodeChannel::UninitHisi(ChannelCount);
    if (!DecodeChannel::InitHisi(ChannelCount)
        ||!EncodeChannel::InitHisi(ChannelCount, DecodeChannel::DestinationWidth,DecodeChannel::DestinationHeight))
    {
        exit(2);
    }

    _socketMaid = new SocketMaid(2,true);
    _handler.HttpReceived.Subscribe(this);
    if (_socketMaid->AddListenEndPoint(EndPoint(7772), &_handler) == -1)
    {
        exit(2);
    }

    //删除临时目录
    Command::Execute(StringEx::Combine("mkdir -p ", TrafficDirectory::TempDir));
    Command::Execute(StringEx::Combine("mkdir -p ", TrafficDirectory::FileDir));
    Command::Execute(StringEx::Combine("rm -rf ", TrafficDirectory::TempDir, "*"));

    //升级数据库
    TrafficData data;
    data.UpdateDb();

    //软件版本
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
        LogPool::Information(LogEvent::System, "初始化国标sdk");
    }
    else
    {
        LogPool::Information(LogEvent::System, "初始化国标sdk失败,返回结果：", gbResult);
    }

    GbParameter gbParameter = data.GetGbPrameter();
    if (gbParameter.ServerIp.empty())
    {
        LogPool::Information(LogEvent::System, "未找到国标配置，将不使用国标视频");
    }
    else
    {
        loginHandler = vas_sdk_login(const_cast<char*>(gbParameter.ServerIp.c_str()), gbParameter.ServerPort, const_cast<char*>(gbParameter.UserName.c_str()), const_cast<char*>(gbParameter.Password.c_str()));
        if (loginHandler >= 0)
        {
            LogPool::Information(LogEvent::System, "国标登陆成功,登陆句柄:",loginHandler);
        }
        else
        {
            LogPool::Information(LogEvent::System, "国标登陆失败,登陆句柄:", loginHandler);
        }
    }
#endif // !_WIN32

    _mqtt = new MqttChannel("127.0.0.1", 1883);
    _mqtt->MqttDisconnected.Subscribe(this);
    _data = new DataChannel();
    _encode = new EncodeChannel(ChannelCount);

    _mqtt->Start();
    _data->Start();
    _encode->Start();

    InitThreads(loginHandler);
    if (_sdkInited)
    {
        for (unsigned int i = 0; i < _recogns.size(); ++i)
        {
            _recogns[i]->Start();
        }
        for (map<int, DetectChannel*>::iterator it= _detects.begin();it!= _detects.end();++it)
        {
            it->second->Start();
        }
        while (true)
        {
            bool sdkReady = true;
#ifndef _WIN32
            for (map<int, DetectChannel*>::iterator it = _detects.begin(); it != _detects.end(); ++it)
            {
                if (!it->second->Inited())
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
    //int day = DateTime::Today().Day();

    //while (!_cancelled)
    //{
    //    string ps = Command::Execute("top -n 1|grep Genos.out");
    //    vector<string> psRows = StringEx::Split(ps, "\n", true);
    //    if (!psRows.empty())
    //    {
    //        vector<string> columns = StringEx::Split(psRows[0], " ", true);
    //        if (columns.size() >= 5)
    //        {
    //            vector<string> datas = StringEx::Split(columns[4], "m", true);
    //            if (!datas.empty())
    //            {
    //                LogPool::Information(LogEvent::Monitor, "内存使用(MB):", datas[0]);
    //            }
    //        }
    //    }
    //    DateTime today = DateTime::Today();
    //    if (day != today.Day())
    //    {
    //        DateTime removeTime = DateTime::ParseTimeStamp(today.TimeStamp() - LogPool::HoldDays() * 24 * 60 * 60 * 1000);
    //        SqliteLogger::RemoveDatas(removeTime);
    //        day = today.Day();
    //    }
    //    this_thread::sleep_for(chrono::minutes(1));
    //}

    _socketMaid->Stop();
    delete _socketMaid;

    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        _decodes[i]->Stop();
        delete _decodes[i];
    }
    for (map<int, DetectChannel*>::iterator it = _detects.begin(); it != _detects.end(); ++it)
    {
        it->second->Stop();
        delete it->second;
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
    DecodeChannel::UninitFFmpeg();
    MqttChannel::Uninit();
    Socket::Uninit();
}