#include "Socket.h"

using namespace std;
using namespace Saitama;

bool Socket::StartUp()
{
#ifdef _WIN32 
	WSADATA wsd;
	int result = WSAStartup(MAKEWORD(2, 2), &wsd);
	if (result == 0)
	{
		return true;
	}
	else
	{
		LogPool::Error("WSAStartup", WSAErrorCode);
		return false;
	}
#else
	signal(SIGPIPE, SIG_IGN);
#endif
	return true;
}

void Socket::CleanUp()
{
#ifdef _WIN32 
	WSACleanup();
#endif
}

int Socket::UdpSocket()
{
#ifdef _WIN32 
	int udpSocket = static_cast<int>(WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED));
#else
	int udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
#endif
	if (udpSocket == -1)
	{
		LogPool::Error("socket", WSAErrorCode);
		return -1;
	}
	else
	{
		return udpSocket;
	}
}

int Socket::MultiCast(const EndPoint& endPoint)
{
	int udpSocket = UdpSocket();
	if (udpSocket == -1)
	{
		return -1;
	}
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = endPoint.NetPort();
	int result = ::bind(udpSocket, (const sockaddr*)&address, sizeof(address));
	if (result == -1)
	{
		Close(udpSocket);
		LogPool::Error("bind", WSAErrorCode, endPoint.ToString());
		return -1;
	}
	else
	{
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = endPoint.NetIp();
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		result = setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
		if (result ==-1)
		{
			LogPool::Error( "setsocket", WSAErrorCode, endPoint.ToString());
			return -1;
		}
		return udpSocket;
	}
}

int Socket::Bind(const EndPoint& endPoint)
{
	int udpSocket = UdpSocket();
	if (udpSocket == -1)
	{
		return -1;
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = endPoint.NetIp();
	address.sin_port = endPoint.NetPort();
	int result = ::bind(udpSocket, (const sockaddr*)&address, sizeof(address));
	if (result == -1)
	{
		Close(udpSocket);
		LogPool::Error("bind", WSAErrorCode, endPoint.ToString());
		return -1;
	}
	else
	{
		return udpSocket;
	}
}

int Socket::Listen(const EndPoint& endPoint)
{

#ifdef _WIN32 
	int tcpSocket = static_cast<int>(WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED));
#else 
	int tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
#endif
	if (tcpSocket == -1)
	{
		LogPool::Error("socket",WSAErrorCode);
		return -1;
	}
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = endPoint.NetIp();
	address.sin_port = endPoint.NetPort();

	int result = ::bind(tcpSocket, (const sockaddr*)&address, sizeof(address));
	if (result == -1)
	{
		LogPool::Error("bind", WSAErrorCode, endPoint.ToString());
		Close(tcpSocket);
		return -1;
	}
	const int Backlog = 10;
	result = listen(tcpSocket, Backlog);
	if (result == -1)
	{
		Close(tcpSocket);
		LogPool::Error("listen", WSAErrorCode, endPoint.ToString());
		return -1;
	}
	else
	{
		return tcpSocket;
	}
}

int Socket::ConnectTcp(const EndPoint& endPoint)
{
#ifdef _WIN32 
	int tcpSocket = static_cast<int>(WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED));
#else 
	int tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
#endif
	if (tcpSocket == -1)
	{
		LogPool::Error("socket", WSAErrorCode);
		return -1;
	}
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = endPoint.NetIp();
	address.sin_port = endPoint.NetPort();

	if (connect(tcpSocket, (sockaddr*)&address, sizeof(address)) == -1)
	{
		Close(tcpSocket);
		return -1;
	}
	else
	{
		return tcpSocket;
	}
}

bool Socket::SendTcp(int socket, const char* buff, unsigned int size)
{
	if (size == 0)
	{
		return false;
	}
	unsigned int written = 0;
	int n = 0;
	while (written != size)
	{
		if ((n = static_cast<int>(send(socket, buff + written, size-written, 0))) <= 0)
		{
			return false;
		}
		written += n;
	}
	return true;
}

bool Socket::SendUdp(int socket, const EndPoint& endPoint, const char* buff, unsigned int size)
{
	if (size == 0)
	{
		return false;
	}

	unsigned int written = 0;
	int n = 0;
	sockaddr_in address;
	address.sin_addr.s_addr = endPoint.NetIp();
	address.sin_port = endPoint.NetPort();
	address.sin_family = AF_INET;
	while (written != size)
	{
		if ((n = static_cast<int>(sendto(socket, buff + written, size - written, 0, (const sockaddr*)&address, sizeof(address)))) <= 0)
		{
			return false;
		}
		written += n;
	}
	return true;
}

void Socket::Close(int socket)
{
#ifdef _WIN32 
	closesocket(socket);
#else 
	close(socket);
#endif
}