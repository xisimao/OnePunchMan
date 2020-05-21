#pragma once
#include "MqttChannel.h"
#include "SocketMaid.h"
#include "FlowData.h"

namespace OnePunchMan
{

	class IoAdapter:public ThreadObject,public IObserver<MqttReceivedEventArgs>
	{
	public:
		IoAdapter();

		void Update(MqttReceivedEventArgs* e);

	protected:

		void StartCore();
	private:

		SocketMaid* _maid;
		MqttChannel* _mqtt;
		SocketHandler _handler;
		std::mutex _mutex;
		std::set<EndPoint> _ips;
		std::map<std::string, FlowLane> _lanes;
	};
}

