#include "EndPoint.h"

using namespace std;
using namespace Saitama;

EndPoint::EndPoint()
	:EndPoint(0,0)
{

}

EndPoint::EndPoint(unsigned short port)
{
	_netIp = 0;
	_netPort = htons(port);
	_hostPort = port;
	InitAddress(_netIp, _netPort);
}

EndPoint::EndPoint(unsigned int ip, unsigned short port)
{
	_netIp = ip;
	_netPort = port;
	_hostPort = ntohs(port);
	InitAddress(_netIp, _netPort);
}


EndPoint::EndPoint(const string& ip, unsigned short port)
{
	sockaddr_in address;
	if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) == 1)
	{
		_netIp = address.sin_addr.s_addr;
		_netPort = htons(port);
		_hostPort = port;
	}
	else
	{
		LogPool::Error("inet_pton", WSAErrorCode, ip,port);
		_netIp = 0;
		_netPort = 0;
		_hostPort = 0;
	}
	InitAddress(_netIp, _netPort);
}

EndPoint::EndPoint(const string& endPoint)
{
	size_t index = endPoint.find(':');
	if (index == string::npos)
	{
		_netIp = 0;
		_netPort = 0;
		_hostPort = 0;
	}
	else
	{
		sockaddr_in address;
		if (inet_pton(AF_INET, endPoint.substr(0, index).c_str(), &address.sin_addr) == 1)
		{
			_netIp = address.sin_addr.s_addr;
		}
		else
		{
			LogPool::Error("inet_pton", WSAErrorCode, endPoint);
			_netIp = 0;
			_netPort = 0;
			_hostPort = 0;
			return;
		}
		unsigned short port = 0;
		if (StringEx::Convert<unsigned short>(endPoint.substr(index + 1, endPoint.size() - index),&port))
		{
			_netPort = htons(port);
			_hostPort = port;
		}
		else
		{
			_netIp = 0;
			_netPort = 0;
			_hostPort = 0;
		}
	}
	InitAddress(_netIp, _netPort);
}

void EndPoint::InitAddress(unsigned int ip, unsigned short port)
{
	sockaddr_in address;
	address.sin_addr.s_addr = ip;
	address.sin_port = port;
	char buff[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &address.sin_addr, buff, sizeof(address)) == NULL)
	{
		LogPool::Error("inet_ntop", WSAErrorCode, ip,port);
		_hostIp = string();
		_address = string();
	}
	else
	{
		stringstream ss;
		ss << buff << ":" << ntohs(port);
		_hostIp = string(buff);
		_address = ss.str();
	}
}

unsigned int EndPoint::NetIp() const
{
	return _netIp;
}

const string& EndPoint::HostIp() const
{
	return _hostIp;
}

unsigned short EndPoint::NetPort() const
{
	return _netPort;
}

unsigned short EndPoint::HostPort() const
{
	return _hostPort;
}

bool EndPoint::Empty() const
{
	return _netIp == 0 && _netPort == 0;
}

const string& EndPoint::ToString() const
{
	return _address;
}

bool EndPoint::operator == (const EndPoint &right) const
{
	return _netIp == right.NetIp() && _netPort == right.NetPort();
}

bool EndPoint::operator != (const EndPoint &right) const
{
	return !(*this == right);
}

bool EndPoint::operator < (const EndPoint &right) const
{
	if (_netIp < right.NetIp())
	{
		return true;
	}
	else if (_netIp > right.NetIp())
	{
		return false;
	}
	else
	{
		return _netPort < right.NetPort();
	}
}

bool EndPoint::operator <= (const EndPoint &right) const
{
	return *this < right||*this==right;
}

EndPoint EndPoint::GetLocalEndPoint(int socket)
{
	struct sockaddr_in address;
	socklen_t size = sizeof(address);
	if (getsockname(socket, (sockaddr*)&address, &size) == -1)
	{
		return EndPoint();
	}
	else
	{
		return EndPoint(address.sin_addr.s_addr, address.sin_port);
	}
}

EndPoint EndPoint::GetRemoteEndPoint(int socket)
{
	struct sockaddr_in address;
	socklen_t size = sizeof(address);
	if (getpeername(socket, (sockaddr*)&address, &size) == -1)
	{
		return EndPoint();
	}
	else
	{
		return EndPoint(address.sin_addr.s_addr, address.sin_port);
	}
}