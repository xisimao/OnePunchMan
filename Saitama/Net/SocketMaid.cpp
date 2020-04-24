#include "SocketMaid.h"

using namespace std;
using namespace Saitama;

SocketMaid::SocketMaid()
	:SocketMaid(4)
{

}

SocketMaid::SocketMaid(unsigned int channelCount)
{
	Socket::StartUp();
	_channelIndex = 0;
	for (unsigned int i = 0; i < channelCount; ++i)
	{
		SocketChannel* channel = new SocketChannel();
		channel->Accepted.Subscribe(this);
		channel->Closed.Subscribe(this);
		_channels.push_back(channel);
	}

	_connection.Connected.Subscribe(this);
}

SocketMaid::~SocketMaid()
{
	Stop();
	for (unsigned int i = 0; i < _channels.size(); ++i)
	{
		//保证所有都停止才可以delete
		delete _channels[i];
	}
	Socket::CleanUp();
}

void SocketMaid::Update(AcceptedEventArgs* e)
{
	EndPoint localEndPoint = EndPoint::GetLocalEndPoint(e->ListenSocket);
	LogPool::Information(LogEvent::Socket,"accept", e->TcpSocket, localEndPoint.ToString(), EndPoint::GetRemoteEndPoint(e->TcpSocket).ToString());
	EndPoint remoteEndPoint = EndPoint::GetRemoteEndPoint(e->TcpSocket);
	SocketChannel* channel = Select();
	if (channel == NULL)
	{
		LogPool::Warning(LogEvent::Socket, "channel is null", e->TcpSocket);
	}
	else
	{
		channel->AddSocket(e->TcpSocket, SocketType::Accept, e->Handler);

		if (e->Handler == NULL)
		{
			LogPool::Warning(LogEvent::Socket, "handler is null", e->TcpSocket);
		}
		else
		{
			SocketItem item;
			item.Channel = channel;
			item.Handler = e->Handler;
			item.Data.Socket = e->TcpSocket;
			item.Data.StartTime = DateTime::Now();
			item.Data.Tag = 0;
			item.Data.Type = SocketType::Accept;
			item.Data.Local = EndPoint::GetLocalEndPoint(e->ListenSocket);
			item.Data.Remote = remoteEndPoint;
			item.Operation = SocketOperation::Add;
			lock_guard<mutex> lck(_socketMutex);
			_sockets.insert(pair<int, SocketItem>(e->TcpSocket, item));
		}
	}
}

void SocketMaid::Update(ConnectedEventArgs* e)
{
	LogPool::Information(LogEvent::Socket, "connect", e->Socket, e->Ep.ToString());
	EndPoint remoteEndPoint = EndPoint::GetRemoteEndPoint(e->Socket);
	SocketChannel* channel = Select();
	if (channel == NULL)
	{
		LogPool::Warning(LogEvent::Socket, "channel is null", e->Socket);
	}
	else
	{
		channel->AddSocket(e->Socket, SocketType::Connect, e->Handler);

		if (e->Handler == NULL)
		{
			LogPool::Warning(LogEvent::Socket, "handler is null", e->Socket);
		}
		else
		{
			SocketItem item;
			item.Channel = channel;
			item.Handler = e->Handler;
			item.Data.Socket = e->Socket;
			item.Data.StartTime = DateTime::Now();
			item.Data.Tag = 0;
			item.Data.Type = SocketType::Connect;
			item.Data.Local = EndPoint::GetLocalEndPoint(e->Socket);
			item.Data.Remote = remoteEndPoint;
			item.Operation = SocketOperation::Add;
			lock_guard<mutex> lck(_socketMutex);
			_sockets.insert(pair<int, SocketItem>(e->Socket, item));
		}
	}
}

void SocketMaid::Update(ClosedEventArgs* e)
{
	RemoveSocket(e->Socket, e->Result);
	//tcp客户端重连
	_connection.ReportTcpError(e->Socket);
}

SocketChannel* SocketMaid::Select()
{
	if (_channels.empty())
	{
		return NULL;
	}

	if (_channelIndex >= _channels.size())
	{
		_channelIndex = 0;
	}

	SocketChannel* channel = _channels[_channelIndex++];
	if (channel == NULL)
	{
		LogPool::Warning(LogEvent::Socket, "not found any socket channel");
	}
	return channel;
}

void SocketMaid::SetTag(int socket, unsigned short tag)
{
	lock_guard<mutex> lck(_socketMutex);
	if (_tags.find(tag) == _tags.end())
	{
		_tags[tag] = socket;
		_sockets[socket].Data.Tag = tag;
		LogPool::Information(LogEvent::Socket, "tag", socket, tag);
	}
	else if(_tags[tag] != socket)
	{
		RemoveSocket(_tags[tag]);
		_tags[tag] = socket;
		_sockets[socket].Data.Tag = tag;
		LogPool::Information(LogEvent::Socket, "tag", socket, tag);
	}

}

int SocketMaid::AddListenEndPoint(const EndPoint& endPoint, SocketHandler* handler)
{
	int socket = Socket::Listen(endPoint);
	if (socket == -1)
	{
		return -1;
	}
	else
	{
		LogPool::Information(LogEvent::Socket, "listen", socket, endPoint.ToString());
		SocketChannel* channel = Select();
		if (channel == NULL)
		{
			LogPool::Warning(LogEvent::Socket, "channel is null", socket);
		}
		else
		{
			channel->AddSocket(socket, SocketType::Listen, handler);
			if (handler == NULL)
			{
				LogPool::Warning(LogEvent::Socket, "handler is null", socket);
			}
			else
			{
				SocketItem item;
				item.Channel = channel;
				item.Handler = handler;
				item.Data.Socket = socket;
				item.Data.StartTime = DateTime::Now();
				item.Data.Tag = 0;
				item.Data.Type = SocketType::Listen;
				item.Data.Local = EndPoint::GetLocalEndPoint(socket);
				lock_guard<mutex> lck(_socketMutex);
				_sockets.insert(pair<int, SocketItem>(socket, item));
			}
		}
		return socket;
	}
}

void SocketMaid::AddConnectEndPoint(const EndPoint& endPoint, SocketHandler* handler)
{
	LogPool::Information(LogEvent::Socket, "add connect", endPoint.ToString());
	_connection.AddTcpEndPoint(endPoint,handler);
}

int SocketMaid::AddUdpSocket(SocketHandler* handler, int channelIndex)
{
	int socket = Socket::UdpSocket();
	if (socket == -1)
	{
		return -1;
	}
	else
	{
		LogPool::Information(LogEvent::Socket, "udp client", socket);
		AddUdpEndPoint(EndPoint(), socket, handler, channelIndex);
		return socket;
	}
}

int SocketMaid::AddBindEndPoint(const EndPoint& endPoint, SocketHandler* handler, int channelIndex)
{
	int socket = Socket::Bind(endPoint);
	if (socket == -1)
	{
		return -1;
	}
	else
	{
		LogPool::Information(LogEvent::Socket, "udp bind", socket, endPoint.ToString());
		AddUdpEndPoint(endPoint, socket, handler, channelIndex);
		return socket;
	}
}

int SocketMaid::AddMultiCastEndPoint(const EndPoint& endPoint, SocketHandler* handler, int channelIndex)
{
	int socket = Socket::MultiCast(endPoint);
	if (socket == -1)
	{
		return -1;
	}
	else
	{
		LogPool::Information(LogEvent::Socket, "multicast", socket, endPoint.ToString());
		AddUdpEndPoint(endPoint, socket, handler, channelIndex);
		return socket;
	}
}

void SocketMaid::AddUdpEndPoint(const EndPoint& endPoint, int socket, SocketHandler* handler, int channelIndex)
{
	if (!endPoint.Empty())
	{
		_connection.AddUdpEndPoint(endPoint, socket);
	}

	SocketChannel* channel = NULL;
	if (channelIndex == -1)
	{
		channel = Select();
	}
	else
	{
		if (!_channels.empty() && static_cast<int>(_channels.size()) > channelIndex)
		{
			channel = _channels[channelIndex];
		}
		else
		{
			channel = NULL;
		}
	}

	if (channel == NULL)
	{
		LogPool::Warning(LogEvent::Socket, "channel is null", socket);
	}
	else
	{
		channel->AddSocket(socket, SocketType::Udp, handler);
		if (handler == NULL)
		{
			LogPool::Warning(LogEvent::Socket, "handler is null", socket);
		}
		else
		{
			SocketItem item;
			item.Channel = channel;
			item.Handler = handler;
			item.Data.Socket = socket;
			item.Data.StartTime = DateTime::Now();
			item.Data.Tag = 0;
			item.Data.Type = SocketType::Udp;
			item.Data.Local = endPoint;
			item.Operation = SocketOperation::Add;
			lock_guard<mutex> lck(_socketMutex);
			_sockets.insert(pair<int, SocketItem>(socket, item));
		}
	}
}

void SocketMaid::RemoveEndPoint(const EndPoint& endPoint)
{
	int socket = _connection.GetSocket(endPoint);
	LogPool::Information(LogEvent::Socket, "remove connect", endPoint.ToString(), socket);
	_connection.RemoveEndPoint(endPoint);
	if (socket > 0)
	{
		RemoveSocket(socket);
	}
}

void SocketMaid::RemoveSocket(int socket, int result)
{
	unique_lock<mutex> socketLock(_socketMutex);
	//删除tag
	map<unsigned short, int>::iterator it = find_if(_tags.begin(), _tags.end(), [socket](map<unsigned short, int>::reference r) {

		return r.second == socket;

	});
	if (it != _tags.end())
	{
		_tags.erase(it);
	}

	if (_sockets.find(socket) == _sockets.end())
	{
		LogPool::Information(LogEvent::Socket, "close", socket, result);
		Socket::Close(socket);
	}
	else
	{
		const SocketData& data = _sockets[socket].Data;
		SocketHandler* handler = _sockets[socket].Handler;
		LogPool::Information(LogEvent::Socket, "close", socket,
			result,
			data.StartTime.ToString(),
			data.Local.ToString(),
			data.Remote.ToString(),
			data.Tag,
			static_cast<int>(data.Type),
			handler->TransmitSize(),
			handler->ReceiveSize());

		_sockets[socket].Channel->RemoveSocket(socket);
		//删除socket
		_sockets.erase(socket);
	}
	socketLock.unlock();
}

SocketResult SocketMaid::SendTcp(int socket, const string& buffer, AsyncHandler* handler)
{
	lock_guard<mutex> lck(_socketMutex);
	if (_sockets.find(socket) == _sockets.end())
	{
		delete handler;
		return SocketResult::NotFoundSocket;
	}
	else
	{
		return _sockets[socket].Handler->SendTcp(socket,buffer, handler);
	}
}

SocketResult SocketMaid::SendTcp(const EndPoint& endPoint, const string& buffer, AsyncHandler* handler)
{
	int socket = _connection.GetSocket(endPoint);
	if (socket == 0)
	{
		delete handler;
		return SocketResult::NotFoundSocket;
	}
	else if (socket == -1)
	{
		delete handler;
		return SocketResult::Disconnection;
	}
	return SendTcp(socket, buffer, handler);
}

SocketResult SocketMaid::SendTcp(unsigned short tag, const string& buffer, AsyncHandler* handler)
{
	if (tag == 0)
	{
		delete handler;
		return SocketResult::InvalidTag;
	}
	lock_guard<mutex> lck(_socketMutex);
	if (_tags.find(tag) == _tags.end())
	{
		delete handler;
		return SocketResult::NotFoundSocket;
	}
	return SendTcp(_tags[tag], buffer, handler);
}

SocketResult SocketMaid::SendTcp(int socket, const string& buffer)
{
	return SendTcp(socket, buffer, static_cast<AsyncHandler*>(NULL));
}

SocketResult SocketMaid::SendTcp(const EndPoint& endPoint, const string& buffer)
{
	return SendTcp(endPoint, buffer, static_cast<AsyncHandler*>(NULL));
}

SocketResult SocketMaid::SendTcp(unsigned short tag, const string& buffer)
{
	return SendTcp(tag, buffer, static_cast<AsyncHandler*>(NULL));
}

SocketResult SocketMaid::SendTcp(int socket, const string& buffer, long long timeStamp, unsigned int protocolId)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp,protocolId);
	SocketResult result = SendTcp(socket, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendTcp(const EndPoint& endPoint, const string& buffer, long long timeStamp, unsigned int protocolId)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp,protocolId);
	SocketResult result = SendTcp(endPoint, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendTcp(unsigned short tag, const string& buffer, long long timeStamp, unsigned int protocolId)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp,protocolId);
	SocketResult result = SendTcp(tag, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendTcp(int socket, const string& buffer, long long timeStamp, unsigned int protocolId,string* responseBuffer)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp, protocolId, responseBuffer);
	SocketResult result = SendTcp(socket, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendTcp(const EndPoint& endPoint, const string& buffer, long long timeStamp, unsigned int protocolId, string* responseBuffer)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp, protocolId, responseBuffer);
	SocketResult result = SendTcp(endPoint, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendTcp(unsigned short tag, const string& buffer, long long timeStamp, unsigned int protocolId, string* responseBuffer)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp, protocolId, responseBuffer);
	SocketResult result = SendTcp(tag, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendUdp(int socket, const EndPoint& remoteEndPoint, const string& buffer,AsyncHandler* handler)
{
	lock_guard<mutex> lck(_socketMutex);
	if (_sockets.find(socket) == _sockets.end())
	{
		delete handler;
		return SocketResult::NotFoundSocket;
	}
	else
	{
		return _sockets[socket].Handler->SendUdp(socket,remoteEndPoint, buffer, handler);
	}
}

SocketResult SocketMaid::SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const string& buffer, AsyncHandler* handler)
{
	int socket = _connection.GetSocket(bindEndPoint);
	if (socket == 0)
	{
		delete handler;
		return SocketResult::NotFoundSocket;
	}
	return SendUdp(socket, remoteEndPoint, buffer,handler);
}

SocketResult SocketMaid::SendUdp(int socket, const EndPoint& remoteEndPoint, const string& buffer)
{
	return SendUdp(socket, remoteEndPoint, buffer, static_cast<AsyncHandler*>(NULL));
}

SocketResult SocketMaid::SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const string& buffer)
{
	return SendUdp(bindEndPoint, remoteEndPoint, buffer, static_cast<AsyncHandler*>(NULL));
}

SocketResult SocketMaid::SendUdp(int socket, const EndPoint& remoteEndPoint, const string& buffer, long long timeStamp, unsigned int protocolId)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp,protocolId);
	SocketResult result = SendUdp(socket, remoteEndPoint, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const string& buffer, long long timeStamp, unsigned int protocolId)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp,protocolId);
	SocketResult result = SendUdp(bindEndPoint, remoteEndPoint, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendUdp(int socket, const EndPoint& remoteEndPoint, const string& buffer, long long timeStamp, unsigned int protocolId, string* responseBuffer)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp,protocolId, responseBuffer);
	SocketResult result = SendUdp(socket, remoteEndPoint, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

SocketResult SocketMaid::SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const string& buffer, long long timeStamp, unsigned int protocolId, string* responseBuffer)
{
	NoticeHandler* handler = new NoticeHandler(timeStamp, protocolId, responseBuffer);
	SocketResult result = SendUdp(bindEndPoint, remoteEndPoint, buffer, handler);
	if (result == SocketResult::Success)
	{
		if (handler->Wait())
		{
			return SocketResult::Success;
		}
		else
		{
			return SocketResult::Timeout;
		}
	}
	else
	{
		return result;
	}
}

void SocketMaid::Start()
{
	for (unsigned int i = 0; i < _channels.size(); ++i)
	{
		_channels[i]->Start();
	}
	_connection.Start();
}

void SocketMaid::Stop()
{
	_connection.Stop();
	for (unsigned int i = 0; i < _channels.size(); ++i)
	{
		_channels[i]->Stop();
	}
	for (map<int, SocketItem>::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
	{
		if (it->second.Data.Type == SocketType::Listen||it->second.Data.Type==SocketType::Udp)
		{
			Socket::Close(it->first);
		}
	}
}
