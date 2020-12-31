#include "EndPoint.h"

using namespace std;
using namespace OnePunchMan;

EndPoint::EndPoint()
	:EndPoint(0, 0)
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
		LogPool::Error(LogEvent::Socket, "inet_pton", WSAErrorCode, ip, port);
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
			LogPool::Error(LogEvent::Socket, "inet_pton", WSAErrorCode, endPoint);
			_netIp = 0;
			_netPort = 0;
			_hostPort = 0;
			return;
		}
		unsigned short port = 0;
		if (StringEx::TryConvert(endPoint.substr(index + 1, endPoint.size() - index), &port))
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
		LogPool::Error(LogEvent::Socket, "inet_ntop", WSAErrorCode, ip, port);
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

bool EndPoint::operator == (const EndPoint& right) const
{
	return _netIp == right.NetIp() && _netPort == right.NetPort();
}

bool EndPoint::operator != (const EndPoint& right) const
{
	return !(*this == right);
}

bool EndPoint::operator < (const EndPoint& right) const
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

bool EndPoint::operator <= (const EndPoint& right) const
{
	return *this < right || *this == right;
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

#ifndef _WIN32

string EndPoint::GetIp(const string& deviceName)
{
	struct ifreq ifr;
	strcpy(ifr.ifr_name, deviceName.c_str());
	ifr.ifr_addr.sa_family = AF_INET;
	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		return string();
	}
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		close(fd);
		return string();
	}
	char value[50] = { 0 };
	strcpy(value, inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	close(fd);
	return string(value);
}

bool EndPoint::SetIp(const string& deviceName, const string& ip)
{
	struct sockaddr	addr;
	struct ifreq ifr;
	((struct sockaddr_in*)&(addr))->sin_family = PF_INET;
	((struct sockaddr_in*)&(addr))->sin_addr.s_addr = inet_addr(ip.c_str());

	ifr.ifr_addr = addr;
	strcpy(ifr.ifr_name, deviceName.c_str());

	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		return false;
	}
	if (ioctl(fd, SIOCSIFADDR, &ifr) != 0)
	{
		return false;
	}
	close(fd);
	return true;
}

string EndPoint::GetMask(const std::string& deviceName)
{
	struct ifreq ifr;
	strcpy(ifr.ifr_name, deviceName.c_str());
	ifr.ifr_addr.sa_family = AF_INET;
	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		return string();
	}
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)
	{
		close(fd);
		return string();
	}
	char value[50] = { 0 };
	strcpy(value, inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	close(fd);
	return string(value);
}

bool EndPoint::SetMask(const string& deviceName, const string& mask)
{
	struct sockaddr	addr;
	struct ifreq ifr;
	((struct sockaddr_in*)&(addr))->sin_family = PF_INET;
	((struct sockaddr_in*)&(addr))->sin_addr.s_addr = inet_addr(mask.c_str());

	ifr.ifr_addr = addr;
	strcpy(ifr.ifr_name, deviceName.c_str());

	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		return false;
	}
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0)
	{
		return false;
	}
	close(fd);
	return true;
}

string EndPoint::GetGateway(const string& deviceName)
{
	char buff[256];
	int  nl = 0;
	struct in_addr gw;
	int flgs, ref, use, metric;
	unsigned long int d, m;
	unsigned int g;
	unsigned long addr = 0;
	unsigned long* pgw = &addr;
	char gateway_addr[50] = { 0 };
	FILE* fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
	{
		return string();
	}

	nl = 0;
	while (fgets(buff, sizeof(buff), fp) != NULL)
	{
		if (nl)
		{
			int ifl = 0;
			while (buff[ifl] != ' ' && buff[ifl] != '\t' && buff[ifl] != '\0')
				ifl++;
			buff[ifl] = 0;    // interface

			if (strcmp(buff, deviceName.c_str()) != 0)
			{
				continue;
			}

			if (sscanf(buff + ifl + 1, "%lx%lx%X%d%d%d%lx", &d, &g, &flgs, &ref, &use, &metric, &m) != 7)
			{
				fclose(fp);
				return string();
			}

			if (flgs == 3)
			{
				gw.s_addr = g;
				*pgw = g;
				strcpy(gateway_addr, inet_ntoa(gw));
				fclose(fp);
				return string(gateway_addr);
			}
		}
		nl++;
	}

	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}

	return string();
}

bool EndPoint::SetGateway(const string& deviceName, const string& gateway)
{
	struct rtentry rt;
	unsigned int gw;
	int fd;

	//删除网关
	string oldGateway = GetGateway(deviceName);
	if (!oldGateway.empty())
	{
		int deleteFd = socket(PF_INET, SOCK_DGRAM, 0);
		if (deleteFd < 0)
		{
			return false;
		}
		unsigned int deleteGw = inet_addr(gateway.c_str());
		struct rtentry deletert;
		memset((char*)&deletert, 0, sizeof(struct rtentry));

		deletert.rt_flags = RTF_UP | RTF_GATEWAY;

		((struct sockaddr_in*)&(deletert.rt_dst))->sin_family = PF_INET;

		((struct sockaddr_in*)&(deletert.rt_gateway))->sin_addr.s_addr = deleteGw;
		((struct sockaddr_in*)&(deletert.rt_gateway))->sin_family = PF_INET;

		((struct sockaddr_in*)&(deletert.rt_genmask))->sin_addr.s_addr = 0;
		((struct sockaddr_in*)&(deletert.rt_genmask))->sin_family = PF_INET;

		deletert.rt_dev = NULL;

		ioctl(deleteFd, SIOCDELRT, &deletert);

		close(deleteFd);
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return false;
	}

	gw = inet_addr(gateway.c_str());
	memset((char*)&rt, 0, sizeof(struct rtentry));

	((struct sockaddr_in*)&(rt.rt_dst))->sin_addr.s_addr = 0;

	rt.rt_flags = RTF_UP | RTF_GATEWAY;

	((struct sockaddr_in*)&(rt.rt_dst))->sin_family = PF_INET;

	((struct sockaddr_in*)&(rt.rt_gateway))->sin_addr.s_addr = gw;
	((struct sockaddr_in*)&(rt.rt_gateway))->sin_family = PF_INET;

	((struct sockaddr_in*)&(rt.rt_genmask))->sin_addr.s_addr = 0;
	((struct sockaddr_in*)&(rt.rt_genmask))->sin_family = PF_INET;
	rt.rt_dev = NULL;

	if (ioctl(fd, SIOCADDRT, &rt) < 0)
	{
		close(fd);
		return false;
	}
	else
	{
		close(fd);
		return true;
	}
}

string EndPoint::GetMac(const string& deviceName)
{
	int sockfd;
	struct 	ifreq ifr;
	char buff[24] = { 0 };
	char mac[50] = { 0 };
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return string();
	}

	strcpy(ifr.ifr_name, deviceName.c_str());
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		close(sockfd);
		return string();
	}
	strcpy(ifr.ifr_name, deviceName.c_str());
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
	{
		close(sockfd);
		return string();
	}
	else
	{
		memcpy(buff, ifr.ifr_hwaddr.sa_data, 8);
		snprintf(mac, sizeof(buff), "%02X:%02X:%02X:%02X:%02X:%02X",
			(buff[0] & 0377), (buff[1] & 0377), (buff[2] & 0377),
			(buff[3] & 0377), (buff[4] & 0377), (buff[5] & 0377));
		close(sockfd);
		return string(mac);
	}
}

#endif // !_WIN32