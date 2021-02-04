#include "Hisi_TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

Hisi_TrafficStartup::Hisi_TrafficStartup()
    :TrafficStartup()
{

}

void Hisi_TrafficStartup::Screenshot(int channelIndex)
{
    
}

void Hisi_TrafficStartup::Start()
{
    LogPool::Information(LogEvent::System, "time zone:", DateTime::TimeZone());

    Socket::Init();
    DecodeChannel::InitFFmpeg();

    EncodeChannel::UninitHisi(ChannelCount);
    DecodeChannel::UninitHisi(ChannelCount);

    if (!DecodeChannel::InitHisi(ChannelCount, 128))
    {
        exit(2);
    }
    if (!EncodeChannel::InitHisi(ChannelCount, DecodeChannel::DestinationWidth, DecodeChannel::DestinationHeight))
    {
        exit(2);
    }
    _socketMaid = new SocketMaid(2, true);
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
            LogPool::Information(LogEvent::System, "国标登陆成功,登陆句柄:", loginHandler);
        }
        else
        {
            LogPool::Information(LogEvent::System, "国标登陆失败,登陆句柄:", loginHandler);
        }
    }
#endif // !_WIN32

    _data = new DataChannel();
    _data->Start();

    //启动解码
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
}
