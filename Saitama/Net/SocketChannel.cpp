#include "SocketChannel.h"

using namespace std;
using namespace Saitama;

#ifndef _WIN32
const int SocketChannel::MaxEvents = 100;
#endif 

const int SocketChannel::Timeout = 100;

const int SocketChannel::BufferLength = 10240;

SocketChannel::SocketChannel()
	:ThreadObject("socket_channel")
{
#ifndef _WIN32 
	_epollfd = epoll_create(MaxEvents);
	if (_epollfd == -1)
	{
		LogPool::Error("epoll_create", WSAErrorCode);
	}
#endif
}

void SocketChannel::AddSocket(int socket,SocketType type, SocketHandler* handler)
{
	if (socket == -1)
	{
		return;
	}
	lock_guard<mutex> lck(_mutex);
	SocketItem data(socket, type, handler, SocketOperation::Add);
	_tempSockets.push_back(data);
}

void SocketChannel::RemoveSocket(int socket)
{
	lock_guard<mutex> lck(_mutex);
	SocketItem data(socket, SocketType::None, NULL, SocketOperation::Remove);
	_tempSockets.push_back(data);
}

void SocketChannel::MoveTempSockets()
{
	if (_tempSockets.empty())
	{
		return;
	}
	lock_guard<mutex> lck(_mutex);
	for_each(_tempSockets.begin(), _tempSockets.end(), [this](SocketItem& data) 
	{
		if (data.Operation == SocketOperation::Add)
		{
#ifndef _WIN32
			epoll_event event;
			event.data.fd = data.Socket;
			event.events = EPOLLIN;
			if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, data.Socket, &event) == -1)
			{
				LogPool::Error("epoll_ctl", WSAErrorCode);
				return;
			}
#endif 
			_sockets.insert(pair<int, SocketItem>(data.Socket, data));
		}
		else if (data.Operation == SocketOperation::Remove)
		{
			if (_sockets.find(data.Socket) != _sockets.end())
			{
				//只有tcp的handler是复制的listen和udp都是传入
				//tcp handler复制只在accpet和connect时触发
				if ((data.Type == SocketType::Accept|| data.Type == SocketType::Connect)&&
					_sockets[data.Socket].Handler != NULL)
				{
					delete _sockets[data.Socket].Handler;
				}
#ifndef _WIN32
				epoll_ctl(_epollfd, EPOLL_CTL_DEL, data.Socket, NULL);
#endif 
				_sockets.erase(data.Socket);
				Socket::Close(data.Socket);
			}
		}
	});

	_tempSockets.clear();
}

void SocketChannel::StartCore()
{

#ifdef _WIN32 
	fd_set fds;
	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = Timeout * 1000;
#else
	struct epoll_event events[MaxEvents];
#endif

	char buffer[BufferLength] = { 0 };
	sockaddr_in remoteAddress;
	socklen_t addressLength = sizeof(remoteAddress);

	while (!Cancelled())
	{
		MoveTempSockets();

		if (_sockets.empty())
		{
			this_thread::sleep_for(chrono::milliseconds(Timeout));
			continue;
		}
#ifdef _WIN32
		FD_ZERO(&fds);
		for_each(_sockets.begin(), _sockets.end(), [&fds](map<int,SocketItem>::reference p) {

			FD_SET(p.first, &fds);
		});
		int count = select(0, &fds, NULL, NULL, &timeout);
		if (count == -1)
		{
			LogPool::Error("select", WSAErrorCode);
			this_thread::sleep_for(chrono::milliseconds(Timeout));
			continue;
		}
		if (count != 0)
		{
			for_each(_sockets.begin(), _sockets.end(), [this, &fds, &buffer, &remoteAddress, &addressLength](map<int, SocketItem>::reference r) {
				int socket = r.first;
				if (FD_ISSET(socket, &fds))
				{
					SocketItem& data = r.second;
#else
		int count = epoll_wait(_epollfd, events, MaxEvents, Timeout);
		if (count == -1)
		{
			LogPool::Error( "epoll_wait", WSAErrorCode);
			continue;
		}
		for (int i = 0; i < count; ++i)
		{
			int socket = events[i].data.fd;

			SocketItem& data = _sockets[socket];
			if (events[i].events&EPOLLIN)
			{
#endif
				switch (data.Type)
				{
				case SocketType::Listen:
				{
					int tcpSocket = static_cast<int>(accept(socket, NULL, NULL));
					if (tcpSocket == -1)
					{
						LogPool::Error("accept", WSAErrorCode, socket);
						break;
					}
					else
					{
						AcceptedEventArgs e;
						e.ListenSocket = socket;
						e.TcpSocket = tcpSocket;
						if (data.Handler == NULL)
						{
							e.Handler = NULL;
						}
						else
						{
							e.Handler = data.Handler->Clone(tcpSocket);
						}
						Accepted.Notice(&e);
						//这里不添加到本通道，由外界根据负载均衡添加
					}
					break;
				}
				case SocketType::Accept:
				case SocketType::Connect:
				{
					int tcpn = static_cast<int>(recv(socket, buffer, BufferLength, 0));
					if (tcpn > 0)
					{
						if (data.Handler != NULL)
						{
							data.Handler->Handle(socket, 0, 0, buffer, tcpn);
						}
					}
					else if (tcpn == 0)
					{
						ClosedEventArgs e;
						e.Socket = socket;
						Closed.Notice(&e);
						RemoveSocket(socket);
					}
					else
					{
						ClosedEventArgs e;
						e.Result = WSAErrorCode;
						e.Socket = socket;
						Closed.Notice(&e);
						RemoveSocket(socket);
					}
					break;
				}
				case SocketType::Udp:
				{
					int udpn = static_cast<int>(recvfrom(socket, buffer, BufferLength, 0, (sockaddr*)&remoteAddress, &addressLength));
					if (udpn > 0)
					{
						if (data.Handler != NULL)
						{
							data.Handler->Handle(socket, remoteAddress.sin_addr.s_addr, remoteAddress.sin_port, buffer, udpn);
						}
					}
					else if(udpn==-1)
					{
						LogPool::Error("recvfrom", WSAErrorCode,EndPoint(remoteAddress.sin_addr.s_addr, remoteAddress.sin_port).ToString());
					}
					break;
				}
				default:
					break;
				}
#ifdef _WIN32
				}
			});
		}
#else
			}
			else
			{
				LogPool::Error("epoll_wait", WSAErrorCode, socket);
				RemoveSocket(socket);
			}
		}
#endif			
	}

	for_each(_sockets.begin(), _sockets.end(), [this](map<int, SocketItem>::reference p) {

		if ((p.second.Type == SocketType::Accept || p.second.Type == SocketType::Connect) &&
			p.second.Handler != NULL)
		{
			delete p.second.Handler;
		}
#ifndef _WIN32
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, p.first, NULL);
#endif 
		Socket::Close(p.first);
	});

}
