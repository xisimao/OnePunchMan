#include "EndPointChannel.h"

using namespace std;
using namespace Saitama;

const int EndPointChannel::ConnectionSpan = 3;

EndPointChannel::EndPointChannel()
	:ThreadObject("connection")
{
}

void EndPointChannel::AddTcpEndPoint(const EndPoint& endPoint, SocketHandler* handler)
{
	unique_lock<mutex> lck(_mutex);
	_tempEndPoints.push_back(EndPointItem(-1,endPoint,SocketType::Connect, handler, SocketOperation::Add));
	_condition.notify_all();
}

void EndPointChannel::AddUdpEndPoint(const EndPoint& endPoint,int socket)
{
	unique_lock<mutex> lck(_mutex);
	_tempEndPoints.push_back(EndPointItem(socket, endPoint, SocketType::Udp,NULL, SocketOperation::Add));
	_condition.notify_all();
}

void EndPointChannel::RemoveEndPoint(const EndPoint& endPoint)
{
	unique_lock<mutex> lck(_mutex);
	_tempEndPoints.push_back(EndPointItem(-1, endPoint,SocketType::None,NULL, SocketOperation::Remove));
	_condition.notify_all();
}

void EndPointChannel::ReportTcpError(int socket)
{
	unique_lock<mutex> lck(_mutex);
	_tempEndPoints.push_back(EndPointItem(socket, EndPoint(),SocketType::None,NULL, SocketOperation::Error));
	_condition.notify_all();
}

int EndPointChannel::GetSocket(const EndPoint& endPoint)
{
	unique_lock<mutex> lck(_mutex);
	if (_endPoints.find(endPoint) == _endPoints.end())
	{
		return 0;
	}
	else
	{
		return _endPoints[endPoint].Socket;
	}
}

void EndPointChannel::MoveTempEndPoints()
{
	if (_tempEndPoints.empty())
	{
		return;
	}
	lock_guard<mutex> lck(_mutex);
	for_each(_tempEndPoints.begin(), _tempEndPoints.end(), [this](EndPointItem& data)
	{
		if (data.Operation == SocketOperation::Add)
		{
			_endPoints.insert(pair<EndPoint, EndPointItem>(data.Ep, data));
		}
		else if (data.Operation == SocketOperation::Remove)
		{
			_endPoints.erase(data.Ep);
		}
		else if (data.Operation == SocketOperation::Error)
		{
			for (map<EndPoint, EndPointItem>::iterator it = _endPoints.begin(); it != _endPoints.end(); ++it)
			{
				if (it->second.Socket == data.Socket&&it->second.Type==SocketType::Connect)
				{
					it->second.Socket = -1;
					break;
				}
			}
		}
	});
	_tempEndPoints.clear();
}

void EndPointChannel::StartCore()
{
	while (!Cancelled())
	{
		MoveTempEndPoints();

		for_each(_endPoints.begin(), _endPoints.end(), [this](map<EndPoint, EndPointItem>::reference r) {
			{
				if (r.second.Socket == -1)
				{
					int socket = Socket::ConnectTcp(r.first);
					if (socket != -1)
					{
						r.second.Socket = socket;
						ConnectedEventArgs e;
						e.Ep = r.first;
						e.Socket = socket;
						e.Handler = r.second.Handler == NULL ? NULL : r.second.Handler->Clone(socket);
						Connected.Notice(&e);
					}
				}
			}
		});
		
		unique_lock<mutex> lck(_mutex);
		if (_tempEndPoints.empty()&&all_of(_endPoints.begin(), _endPoints.end(), [](map<EndPoint, EndPointItem>::reference p) {return p.second.Socket != -1; }))
		{
			_condition.wait(lck);
		}
		lck.unlock();
		this_thread::sleep_for(chrono::seconds(ConnectionSpan));
	}
}

void EndPointChannel::StopCore()
{
	_condition.notify_all();
}