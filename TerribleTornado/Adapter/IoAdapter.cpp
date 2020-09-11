#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;


IoAdapter::IoAdapter()
	:ThreadObject("IO"), _maid(NULL),_mqtt(NULL)
{

}

void IoAdapter::Update(MqttReceivedEventArgs* e)
{
	JsonDeserialization jd(e->Message);
	int index = 0;
	while (true)
	{
		string channelUrl = jd.Get<string>(StringEx::Combine(":", index, ":channelUrl"));
		if (channelUrl.empty())
		{
			break;
		}
		string laneId = jd.Get<string>(StringEx::Combine(":", index, ":laneId"));
		int status =  jd.Get<int>(StringEx::Combine(":", index, ":status"));
		lock_guard<mutex> lck(_mutex);
		map<string, FlowLane>::iterator it = _lanes.find(channelUrl + "_" + laneId);
		if (it != _lanes.end())
		{
			EndPoint ep(it->second.IOIp, 24000);
			if (_ips.find(ep) != _ips.end())
			{
				std::stringstream ss;
				ss << "|set|"
					<< setw(2) << setfill('0') << (it->second.IOIndex-1)
					<< ":"
					<< status
					<< "|";
				SocketResult r=_maid->SendTcp(ep, ss.str());
				LogPool::Information(LogEvent::Adapter, static_cast<int>(r), ss.str());
			}
		}
		index += 1;
	}
}

void IoAdapter::StartCore()
{
	Socket::Init();
	MqttChannel::Init();
	_maid = new SocketMaid(0,true);
	vector<string> topics;
	topics.push_back("IO");
	_mqtt = new MqttChannel("127.0.0.1", 1883, topics);
	_mqtt->MqttReceived.Subscribe(this);
	_maid->Start();
	_mqtt->Start();
	int minute = DateTime::Now().Minute()-1;
	while (!_cancelled)
	{
		DateTime now = DateTime::Now();
		if (minute == now.Minute())
		{
			this_thread::sleep_for(chrono::seconds(1));
		}
		else
		{
			minute = now.Minute();
			FlowChannelData data;
			vector<FlowChannel> channels=data.GetList();
			lock_guard<mutex> lck(_mutex);
			_lanes.clear();
			set<EndPoint> ips;
			for (vector<FlowChannel>::iterator cit=channels.begin(); cit !=channels.end();++cit)
			{
				for (vector<FlowLane>::iterator lit=cit->Lanes.begin();lit!=cit->Lanes.end();++lit)
				{
					if (!lit->IOIp.empty())
					{
						ips.insert(EndPoint(lit->IOIp, 24000));
						_lanes.insert(pair<string, FlowLane>(cit->ChannelUrl + "_" + lit->LaneId, *lit));
					}
				}
			}
			for (set<EndPoint>::iterator it = _ips.begin(); it != _ips.end();)
			{
				if (ips.find(*it) == ips.end())
				{
					_maid->RemoveEndPoint(*it);
					it = _ips.erase(it);
				}
				else
				{
					++it;
				}
			}
			for (set<EndPoint>::iterator it = ips.begin(); it != ips.end(); ++it)
			{
				if (_ips.find(*it) == _ips.end())
				{
					_maid->AddConnectEndPoint(*it,NULL);
					_ips.insert(*it);
				}
			}
		}
	}
	_maid->Stop();
	delete _maid;
	_mqtt->Stop();
	delete _mqtt;
	MqttChannel::Uninit();
	Socket::Uninit();
}