#include "DG_TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;

DG_TrafficStartup::DG_TrafficStartup()
    :TrafficStartup()
{

}

void DG_TrafficStartup::UpdateChannel(const TrafficChannel& channel)
{
    TrafficStartup::UpdateChannel(channel);
    _handlers[channel.ChannelIndex - 1]->UpdateChannel(channel);
}

void DG_TrafficStartup::Screenshot(int channelIndex)
{
    _handlers[channelIndex - 1]->WriteBmp();
}

void DG_TrafficStartup::Start()
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

    //ɾ����ʱĿ¼
    Command::Execute(StringEx::Combine("mkdir -p ", TrafficDirectory::TempDir));
    Command::Execute(StringEx::Combine("mkdir -p ", TrafficDirectory::FileDir));
    Command::Execute(StringEx::Combine("rm -rf ", TrafficDirectory::TempDir, "*"));

    //�������ݿ�
    TrafficData data;
    data.UpdateDb();

    //����汾
    _softwareVersion = data.GetParameter("Version");

    //��ʼ��sdk
    DG_FrameHandler::Init();

    //��ʼ������
    int loginHandler = -1;
#ifndef _WIN32
    int gbResult = vas_sdk_startup();
    if (gbResult >= 0)
    {
        LogPool::Information(LogEvent::System, "��ʼ������sdk");
    }
    else
    {
        LogPool::Information(LogEvent::System, "��ʼ������sdkʧ��,���ؽ����", gbResult);
    }

    GbParameter gbParameter = data.GetGbPrameter();
    if (gbParameter.ServerIp.empty())
    {
        LogPool::Information(LogEvent::System, "δ�ҵ��������ã�����ʹ�ù�����Ƶ");
    }
    else
    {
        loginHandler = vas_sdk_login(const_cast<char*>(gbParameter.ServerIp.c_str()), gbParameter.ServerPort, const_cast<char*>(gbParameter.UserName.c_str()), const_cast<char*>(gbParameter.Password.c_str()));
        if (loginHandler >= 0)
        {
            LogPool::Information(LogEvent::System, "�����½�ɹ�,��½���:", loginHandler);
        }
        else
        {
            LogPool::Information(LogEvent::System, "�����½ʧ��,��½���:", loginHandler);
        }
    }
#endif // !_WIN32

    _data = new DataChannel();
    _data->Start();

    //��ʼ�����
    for (int i = 0; i < ChannelCount; ++i)
    {
        FlowDetector* flowDetector = new FlowDetector(DG_DecodeChannel::DestinationWidth, DG_DecodeChannel::DestinationHeight, _socketMaid, _data);
        _flowDetectors.push_back(flowDetector);
        EventDetector* eventDetector = new EventDetector(DG_DecodeChannel::DestinationWidth, DG_DecodeChannel::DestinationHeight, _encode, _data);
        _eventDetectors.push_back(eventDetector);
        DG_FrameHandler* handler = new DG_FrameHandler(i + 1, DG_DecodeChannel::DestinationWidth, DG_DecodeChannel::DestinationHeight, flowDetector);
        DG_DecodeChannel* decode = new DG_DecodeChannel(i + 1, loginHandler, NULL, handler);
        _decodes.push_back(decode);
        _handlers.push_back(handler);
    }

    //��������
    for (unsigned int i = 0; i < _decodes.size(); ++i)
    {
        _decodes[i]->Start();
    }

    //��ʼ��ͨ��
    set<EndPoint> ips;
    for (int i = 0; i < ChannelCount; ++i)
    {
        TrafficChannel channel = data.GetChannel(i + 1);
        if (!channel.ChannelUrl.empty())
        {
            //ioת������ַ
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

    //��ʼ��ioת����
    for (set<EndPoint>::iterator it = ips.begin(); it != ips.end(); ++it)
    {
        LogPool::Information(LogEvent::System, "���ӵ�ioת����", it->HostIp());
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
    //                LogPool::Information(LogEvent::Monitor, "�ڴ�ʹ��(MB):", datas[0]);
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
