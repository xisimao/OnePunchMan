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
    _sdkInited = Seemmo_SDK::Init();
    if (Seemmo_SDK::seemmo_version != NULL)
    {
        _sdkVersion = Seemmo_SDK::seemmo_version();
    }

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
            LogPool::Information(LogEvent::System, "�����½�ɹ�,��½���:",loginHandler);
        }
        else
        {
            LogPool::Information(LogEvent::System, "�����½ʧ��,��½���:", loginHandler);
        }
    }
#endif // !_WIN32

    //��ʼ�����ݿ����
    _data = new DataChannel();
    _data->Start();

    //��ʼ������
    _encode = new EncodeChannel(ChannelCount);
    _encode->Start();

    //��ʼ�����
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

    //�������ʶ���߳�
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
   
    //���������߳�
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