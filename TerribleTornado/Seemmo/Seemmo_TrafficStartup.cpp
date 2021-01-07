#include "Seemmo_TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

const int Seemmo_TrafficStartup::DetectCount = 4;
const int Seemmo_TrafficStartup::RecognCount = 2;

Seemmo_TrafficStartup::Seemmo_TrafficStartup()
    :TrafficStartup()
{
}

void Seemmo_TrafficStartup::UpdateChannel(const TrafficChannel& channel)
{
    TrafficStartup::UpdateChannel(channel);
    _detects[channel.ChannelIndex]->UpdateChannel(channel);
}

void Seemmo_TrafficStartup::Screenshot(int channelIndex)
{
    _detects[channelIndex]->Screenshot(channelIndex);
}

void Seemmo_TrafficStartup::Start()
{
    Socket::Init();
    Seemmo_DecodeChannel::InitFFmpeg();

    Seemmo_DecodeChannel::UninitHisi(ChannelCount);
    if (!Seemmo_DecodeChannel::InitHisi(ChannelCount,64)
        ||!EncodeChannel::InitHisi(ChannelCount, Seemmo_DecodeChannel::DestinationWidth,Seemmo_DecodeChannel::DestinationHeight))
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
    _sdkInited = Seemmo_SDK::Init();
    if (Seemmo_SDK::seemmo_version != NULL)
    {
        _sdkVersion = Seemmo_SDK::seemmo_version();
    }

    //初始化国标
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

    //初始化数据库入库
    _data = new DataChannel();
    _data->Start();

    //初始化编码
    _encode = new EncodeChannel(ChannelCount);
    _encode->Start();

    //初始化组件
    vector<Seemmo_DecodeChannel*> decodes;
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowDetector* flowDetector = new FlowDetector(Seemmo_DecodeChannel::DestinationWidth, Seemmo_DecodeChannel::DestinationHeight, _socketMaid, _data);
        _flowDetectors.push_back(flowDetector);
        EventDetector* eventDetector = new EventDetector(Seemmo_DecodeChannel::DestinationWidth, Seemmo_DecodeChannel::DestinationHeight, _encode, _data);
        _eventDetectors.push_back(eventDetector);
        Seemmo_DecodeChannel* decode = new Seemmo_DecodeChannel(i + 1, loginHandler, _encode);
        _decodes.push_back(decode);
        decodes.push_back(decode);
    }
    for (int i = 0; i < RecognCount; ++i)
    {
        Seemmo_RecognChannel* recogn = new Seemmo_RecognChannel(i, Seemmo_DecodeChannel::DestinationWidth, Seemmo_DecodeChannel::DestinationHeight, &_flowDetectors);
        _recogns.push_back(recogn);
    }
    vector<Seemmo_DetectChannel*> detects;
    for (int i = 0; i < DetectCount; ++i)
    {
        Seemmo_DetectChannel* detect = new Seemmo_DetectChannel(i, Seemmo_DecodeChannel::DestinationWidth, Seemmo_DecodeChannel::DestinationHeight);
        detects.push_back(detect);
    }
    for (int i = 0; i < DetectCount; ++i)
    {
        detects.at(i)->SetRecogn(_recogns.at(i % RecognCount));
        for (int j = 0; j < ChannelCount / DetectCount; ++j)
        {
            int channelIndex = i + (j * DetectCount) + 1;
            detects.at(i)->AddChannel(channelIndex, decodes.at(channelIndex - 1), _flowDetectors.at(channelIndex - 1), _eventDetectors.at(channelIndex - 1));
            _detects.insert(pair<int, Seemmo_DetectChannel*>(channelIndex, detects.at(i)));
        }
    }

    //启动检测识别线程
    if (_sdkInited)
    {
        for (unsigned int i = 0; i < _recogns.size(); ++i)
        {
            _recogns[i]->Start();
        }
        for (map<int, Seemmo_DetectChannel*>::iterator it= _detects.begin();it!= _detects.end();++it)
        {
            it->second->Start();
        }
        while (true)
        {
            bool sdkReady = true;
#ifndef _WIN32
            for (map<int, Seemmo_DetectChannel*>::iterator it = _detects.begin(); it != _detects.end(); ++it)
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
   
    //启动解码线程
    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        _decodes[i]->Start();
    }

    //初始化通道
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
            UpdateChannel(channel);
        }
    }

    //初始化io转换器
    for (set<EndPoint>::iterator it = ips.begin(); it != ips.end(); ++it)
    {
        LogPool::Information(LogEvent::System, "连接到io转换器", it->HostIp());
        _socketMaid->AddConnectEndPoint(*it, NULL);
    }
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
    for (map<int, Seemmo_DetectChannel*>::iterator it = _detects.begin(); it != _detects.end(); ++it)
    {
        it->second->Stop();
        delete it->second;
    }

    for (unsigned int i = 0; i < _recogns.size(); ++i)
    {
        _recogns[i]->Stop();
        delete _recogns[i];
    }

    Seemmo_SDK::Uninit();
    EncodeChannel::UninitHisi(ChannelCount);
    Seemmo_DecodeChannel::UninitHisi(ChannelCount);
    Seemmo_DecodeChannel::UninitFFmpeg();
    Socket::Uninit();
}