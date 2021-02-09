#include "TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

const int TrafficStartup::ChannelCount = 8;

TrafficStartup::TrafficStartup()
    :_startTime(DateTime::Now()), _sdkInited(false), _socketMaid(NULL)
    , _data(NULL), _encode(NULL)
{
    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    FlowDetector::Init(jd);
    EventDetector::Init(jd);
    DataChannel::Init(jd);
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
            GbParameter parameter = data.GetGbPrameter();
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
            for (vector<GbDevice>::iterator it = devices.begin(); it != devices.end(); ++it)
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
            vector<GbChannel> channels = data.GetGbChannels(gbId);
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
        if (CheckChannelIndex(channelIndex))
        {
            Screenshot(channelIndex);
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
        string sn = jd.Get<string>("sn");
        TrafficData data;
        data.SetParameter("SN", sn);
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/flow/data"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            string channelUrl;
            string laneId;
            DateTime startTime;
            DateTime endTime;
            int pageNum = 0;
            int pageSize = 0;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelurl") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelUrl = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("laneid") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            laneId = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("starttime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            startTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("endtime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            endTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("pagenum") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageNum = StringEx::Convert<int>(pair[1]);
                        }

                    }
                    else if (pair[0].compare("pagesize") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageSize = StringEx::Convert<int>(pair[1]);
                        }
                    }
                }
            }
            TrafficData data;
            tuple< vector<FlowData>, int> t = data.GetFlowDatas(channelUrl, laneId, startTime.TimeStamp(), endTime.TimeStamp(), pageNum, pageSize);
            string datasJson;
            for (vector<FlowData>::iterator it = get<0>(t).begin(); it != get<0>(t).end(); ++it)
            {
                JsonSerialization::AddClassItem(&datasJson, it->ToJson());
            }
            JsonSerialization::SerializeValue(&e->ResponseJson, "total", get<1>(t));
            JsonSerialization::SerializeArray(&e->ResponseJson, "datas", datasJson);
        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/flow/report"))
    {
        string id = GetId(e->Url, "/api/report");
        int channelIndex = StringEx::Convert<int>(id);
        if (CheckChannelIndex(channelIndex))
        {
            _flowDetectors[channelIndex - 1]->GetReportJson(&e->ResponseJson);
            e->Code = HttpCode::OK;
        }
        else
        {
            e->Code = HttpCode::NotFound;
        }
    }
    else if (UrlStartWith(e->Url, "/api/event/data"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            string channelUrl;
            int type=0;
            DateTime startTime;
            DateTime endTime;
            int pageNum = 0;
            int pageSize = 0;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelurl") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelUrl = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("type") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            type = StringEx::Convert<int>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("starttime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            startTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("endtime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            endTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("pagenum") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageNum = StringEx::Convert<int>(pair[1]);
                        }

                    }
                    else if (pair[0].compare("pagesize") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageSize = StringEx::Convert<int>(pair[1]);
                        }
                    }
                }
            }
            TrafficData data;
            tuple<vector<EventData>, int> t = data.GetEventDatas(channelUrl, type,startTime.TimeStamp(), endTime.TimeStamp(), pageNum, pageSize);
            string datasJson;
            for (vector<EventData>::iterator it = get<0>(t).begin(); it != get<0>(t).end(); ++it)
            {
                JsonSerialization::AddClassItem(&datasJson, it->ToJson(e->Host));
            }
            JsonSerialization::SerializeValue(&e->ResponseJson, "total", get<1>(t));
            JsonSerialization::SerializeArray(&e->ResponseJson, "datas", datasJson);
        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/event/statistics"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            string channelUrl;
            int type = 0;
            DateTime startTime;
            DateTime endTime;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelurl") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelUrl = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("type") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            type = StringEx::Convert<int>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("starttime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            startTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("endtime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            endTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                }
            }
            TrafficData data;
            vector<EventStatistics> datas = data.GetEventStatistics(channelUrl, type, startTime.TimeStamp(), endTime.TimeStamp());
            for (vector<EventStatistics>::iterator it = datas.begin(); it != datas.end(); ++it)
            {
                JsonSerialization::AddClassItem(&e->ResponseJson, it->ToJson());
            }
        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/videostruct/vehicle"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            string channelUrl;
            string laneId;
            int carType = 0;
            DateTime startTime;
            DateTime endTime;
            int pageNum = 0;
            int pageSize = 0;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelurl") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelUrl = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("laneid") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            laneId = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("cartype") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            carType = StringEx::Convert<int>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("starttime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            startTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("endtime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            endTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("pagenum") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageNum = StringEx::Convert<int>(pair[1]);
                        }

                    }
                    else if (pair[0].compare("pagesize") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageSize = StringEx::Convert<int>(pair[1]);
                        }
                    }
                }
            }
            TrafficData data;
            tuple< vector<VehicleData>, int> t = data.GetVehicleDatas(channelUrl, laneId, carType, startTime.TimeStamp(), endTime.TimeStamp(), pageNum, pageSize);
            string datasJson;
            for (vector<VehicleData>::iterator it = get<0>(t).begin(); it != get<0>(t).end(); ++it)
            {
                JsonSerialization::AddClassItem(&datasJson, it->ToJson(e->Host));
            }
            JsonSerialization::SerializeValue(&e->ResponseJson, "total", get<1>(t));
            JsonSerialization::SerializeArray(&e->ResponseJson, "datas", datasJson);
        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/videostruct/bike"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            string channelUrl;
            string laneId;
            int bikeType = 0;
            DateTime startTime;
            DateTime endTime;
            int pageNum = 0;
            int pageSize = 0;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelurl") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelUrl = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("laneid") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            laneId = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("biketype") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            bikeType = StringEx::Convert<int>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("starttime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            startTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("endtime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            endTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("pagenum") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageNum = StringEx::Convert<int>(pair[1]);
                        }

                    }
                    else if (pair[0].compare("pagesize") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageSize = StringEx::Convert<int>(pair[1]);
                        }
                    }
                }
            }
            TrafficData data;
            tuple< vector<BikeData>, int> t = data.GetBikeDatas(channelUrl, laneId,bikeType, startTime.TimeStamp(), endTime.TimeStamp(), pageNum, pageSize);
            string datasJson;
            for (vector<BikeData>::iterator it = get<0>(t).begin(); it != get<0>(t).end(); ++it)
            {
                JsonSerialization::AddClassItem(&datasJson, it->ToJson(e->Host));
            }
            JsonSerialization::SerializeValue(&e->ResponseJson, "total", get<1>(t));
            JsonSerialization::SerializeArray(&e->ResponseJson, "datas", datasJson);
        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/videostruct/pedestrain"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            string channelUrl;
            string laneId;
            DateTime startTime;
            DateTime endTime;
            int pageNum = 0;
            int pageSize = 0;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelurl") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelUrl = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("laneid") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            laneId = StringEx::Convert<string>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("starttime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            startTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("endtime") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            endTime = DateTime::ParseString("%d-%d-%d %d:%d:%d", pair[1]);
                        }
                    }
                    else if (pair[0].compare("pagenum") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageNum = StringEx::Convert<int>(pair[1]);
                        }

                    }
                    else if (pair[0].compare("pagesize") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            pageSize = StringEx::Convert<int>(pair[1]);
                        }
                    }
                }
            }
            TrafficData data;
            tuple< vector<PedestrainData>, int> t = data.GetPedestrainDatas(channelUrl, laneId, startTime.TimeStamp(), endTime.TimeStamp(), pageNum, pageSize);
            string datasJson;
            for (vector<PedestrainData>::iterator it = get<0>(t).begin(); it != get<0>(t).end(); ++it)
            {
                JsonSerialization::AddClassItem(&datasJson, it->ToJson(e->Host));
            }
            JsonSerialization::SerializeValue(&e->ResponseJson, "total", get<1>(t));
            JsonSerialization::SerializeArray(&e->ResponseJson, "datas", datasJson);
        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/io/data"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            int channelIndex = 0;
            string laneId;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelindex") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelIndex = StringEx::Convert<int>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("laneid") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            laneId = StringEx::Convert<string>(pair[1]);
                        }
                    }
                }
            }
            vector<IoData> ioDatas;
            if (channelIndex == 0)
            {
                for (unsigned int i = 0; i < _flowDetectors.size(); ++i)
                {
                    vector<IoData> tempIoDatas = _flowDetectors[i]->GetIoDatas("");
                    ioDatas.insert(ioDatas.end(), tempIoDatas.begin(), tempIoDatas.end());
                }
            }
            else
            {
                if (!CheckChannelIndex(channelIndex))
                {
                    JsonSerialization::SerializeValue(&e->ResponseJson, "message", WStringEx::Combine(L"\x901A\x9053\x5E8F\x53F7\x5FC5\x987B\x5728 1-", ChannelCount, L" \x4E4B\x95F4"));
                    e->Code = HttpCode::BadRequest;
                    return;
                }
                ioDatas= _flowDetectors[channelIndex - 1]->GetIoDatas(laneId);
            }
            for (vector<IoData>::iterator it = ioDatas.begin(); it != ioDatas.end(); ++it)
            {
                JsonSerialization::AddClassItem(&e->ResponseJson, it->ToJson());
            }

        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/io/status"))
    {
        vector<string> datas = StringEx::Split(e->Url, "?", true);
        if (datas.size() > 1)
        {
            int channelIndex = 0;
            string laneId;
            vector<string> params = StringEx::Split(datas[1], "&", true);
            for (vector<string>::iterator mit = params.begin(); mit != params.end(); ++mit)
            {
                vector<string> pair = StringEx::Split(*mit, "=", true);
                if (pair.size() >= 1)
                {
                    if (pair[0].compare("channelindex") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            channelIndex = StringEx::Convert<int>(pair[1]);
                        }
                    }
                    else if (pair[0].compare("laneid") == 0)
                    {
                        if (pair.size() >= 2)
                        {
                            laneId = StringEx::Convert<string>(pair[1]);
                        }
                    }
                }
            }
            vector<IoData> ioDatas;
            if (channelIndex == 0)
            {
                for (unsigned int i = 0; i < _flowDetectors.size(); ++i)
                {
                    vector<IoData> tempIoDatas = _flowDetectors[i]->GetIoStatus(laneId);
                    ioDatas.insert(ioDatas.end(), tempIoDatas.begin(), tempIoDatas.end());
                }
            }
            else
            {
                if (!CheckChannelIndex(channelIndex))
                {
                    JsonSerialization::SerializeValue(&e->ResponseJson, "message", WStringEx::Combine(L"\x901A\x9053\x5E8F\x53F7\x5FC5\x987B\x5728 1-", ChannelCount, L" \x4E4B\x95F4"));
                    e->Code = HttpCode::BadRequest;
                    return;
                }
                ioDatas = _flowDetectors[channelIndex - 1]->GetIoStatus(laneId);
            }
            for (vector<IoData>::iterator it = ioDatas.begin(); it != ioDatas.end(); ++it)
            {
                JsonSerialization::AddClassItem(&e->ResponseJson, it->ToJson());
            }

        }
        e->Code = HttpCode::OK;
    }
    else if (UrlStartWith(e->Url, "/api/account/login"))
    {
        JsonDeserialization jd(e->RequestJson);
        string userName = jd.Get<string>("userName");
        string password = jd.Get<string>("password");
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
            JsonSerialization::SerializeValue(&e->ResponseJson, "dateTime", DateTime::Now().ToString());
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
    string cat = Command::Execute(StringEx::Combine("cat ", TrafficDirectory::WebConfigPath));
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
    JsonSerialization::SerializeValue(&deviceJson, "deviceTime", DateTime::Now().ToString());
    JsonSerialization::SerializeValue(&deviceJson, "startTime", _startTime.ToString());
    JsonSerialization::SerializeValue(&deviceJson, "diskUsed", diskUsed);
    JsonSerialization::SerializeValue(&deviceJson, "diskTotal", diskTotal);
    JsonSerialization::SerializeValue(&deviceJson, "licenceStatus", _sdkInited);
    JsonSerialization::SerializeValue(&deviceJson, "sn", sn);
    JsonSerialization::SerializeValue(&deviceJson, "guid", guid);
    JsonSerialization::SerializeValue(&deviceJson, "softwareVersion", _softwareVersion);
    JsonSerialization::SerializeValue(&deviceJson, "webVersion", webVersion);
    JsonSerialization::SerializeValue(&deviceJson, "sdkVersion", _sdkVersion);
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
                ClearChannel(i + 1);
            }
            else
            {
                UpdateChannel(it->second);
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
        UpdateChannel(channel);
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
        ClearChannel(channelIndex);
        return true;
    }
    else
    {
        return false;
    }
}

void TrafficStartup::ClearChannel(int channelIndex)
{
    _decodes[channelIndex-1]->ClearChannel();
    _flowDetectors[channelIndex - 1]->ClearChannel();
    _eventDetectors[channelIndex - 1]->ClearChannel();
}

bool TrafficStartup::CheckChannelIndex(int channelIndex)
{
    return channelIndex >= 1 && channelIndex <= ChannelCount;
}

string TrafficStartup::CheckChannel(TrafficChannel* channel)
{
    if (!CheckChannelIndex(channel->ChannelIndex))
    {
        string json;
        JsonSerialization::SerializeValue(&json, "message", WStringEx::Combine(L"\x901A\x9053\x5E8F\x53F7\x5FC5\x987B\x5728 1-", ChannelCount, L" \x4E4B\x95F4"));
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
        if (!flowDetectLine.Empty() && flowRegion.Empty())
        {
            string json;
            JsonSerialization::SerializeValue(&json, "message", WStringEx::ToString(L"\x6D41\x91CF\x68C0\x6D4B\x533A\x57DF\x6CA1\x6709\x95ED\x5408"));
            return json;
        }
        Polygon queueRegion = Polygon::Build(laneLine1, laneLine2, stopLine, queueDetectLine);
        if (!queueDetectLine.Empty() && queueRegion.Empty())
        {
            string json;
            JsonSerialization::SerializeValue(&json, "message", WStringEx::ToString(L"\x6392\x961F\x68C0\x6D4B\x533A\x57DF\x6CA1\x6709\x95ED\x5408"));
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
                JsonSerialization::SerializeValue(&json, "message", WStringEx::Combine(L"\x7B2C", it->ChannelIndex, L"\x4E2A\x901A\x9053\x7684\x7B2C", it->LaneIndex, L"\x4E2A\x8F66\x9053\x914D\x7F6E\x7684\x68C0\x6D4B\x9879\x4E0E\x7B2C", lit->ChannelIndex, L"\x4E2A\x901A\x9053\x7684\x7B2C", lit->LaneIndex, L"\x4E2A\x8F66\x9053\x91CD\x590D"));
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

void TrafficStartup::UpdateChannel(const TrafficChannel& channel)
{
    unsigned char taskId = _decodes[channel.ChannelIndex-1]->UpdateChannel(channel);
    _flowDetectors[channel.ChannelIndex - 1]->UpdateChannel(taskId,channel);
    _eventDetectors[channel.ChannelIndex - 1]->UpdateChannel(taskId, channel);
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
            && (url[key.size()] == '/' || url[key.size()] == '?');
    }
    else
    {
        return false;
    }
}

string TrafficStartup::GetId(const string& url, const string& key)
{
    return url.size() > key.size() ? url.substr(key.size() + 1, url.size() - key.size() - 1) : string();
}

void TrafficStartup::FillChannel(TrafficChannel* channel, const JsonDeserialization& jd)
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
    if (!channel.ChannelUrl.empty() && CheckChannelIndex(channel.ChannelIndex))
    {
        channelJson = channel.ToJson();
        JsonSerialization::SerializeValue(&channelJson, "rtmpUrl", channel.RtmpUrl(host));
        JsonSerialization::SerializeValue(&channelJson, "flvUrl", channel.FlvUrl(host));
        JsonSerialization::SerializeValue(&channelJson, "channelStatus", static_cast<int>(_decodes[channel.ChannelIndex - 1]->Status()));
        JsonSerialization::SerializeValue(&channelJson, "frameSpan", static_cast<int>(_decodes[channel.ChannelIndex - 1]->FrameSpan()));
        JsonSerialization::SerializeValue(&channelJson, "sourceWidth", static_cast<int>(_decodes[channel.ChannelIndex - 1]->SourceWidth()));
        JsonSerialization::SerializeValue(&channelJson, "sourceHeight", static_cast<int>(_decodes[channel.ChannelIndex - 1]->SourceHeight()));
        JsonSerialization::SerializeValue(&channelJson, "destinationWidth", DecodeChannel::DestinationWidth);
        JsonSerialization::SerializeValue(&channelJson, "destinationHeight", DecodeChannel::DestinationHeight);
        JsonSerialization::SerializeValue(&channelJson, "lanesInited", _flowDetectors[channel.ChannelIndex - 1]->LanesInited());
    }
    return channelJson;
}
