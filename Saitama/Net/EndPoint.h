#pragma once
#include <string>
#include <sstream>

#ifdef _WIN32 
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <linux/route.h>
#include <sys/ioctl.h>    
#endif

#include "LogPool.h"

namespace OnePunchMan
{
#ifdef _WIN32 
#define WSAErrorCode WSAGetLastError()
#else
#define WSAErrorCode errno
#endif // _WIN32

	// IP版本4的IP地址和端口号 
	class EndPoint
	{
	public:

		/**
		* 构造函数
		*/
		EndPoint();

		/**
		* 构造函数
		* @param port 端口的主机端表示方式
		*/
		EndPoint(unsigned short port);

		/**
		* 构造函数
		* @param ip ip的数字表示方式
		* @param port 端口的网络端表示方式
		*/
		EndPoint(unsigned int ip, unsigned short port);

		/**
		* 构造函数
		* @param ip ip的文字表示方式
		* @param port 端口的主机端表示方式
		*/
		EndPoint(const std::string& ip, unsigned short port);

		/**
		* 构造函数
		* @param ip端口的文字表示方式,例如:127.0.0.1:8080
		*/
		EndPoint(const std::string& endPoint);

		/**
		*  获取ip的数字表示方式
		* @return ip的数字表示方式
		*/
		unsigned int NetIp() const;

		/**
		*  获取ip的字符串表示方式
		* @return ip的字符串表示方式
		*/
		const std::string& HostIp() const;

		/**
		*  获取端口的网络端表示方式
		* @return 端口的网络端表示方式
		*/
		unsigned short NetPort() const;

		/**
		*  获取端口的主机端表示方式
		* @return 端口的主机端表示方式
		*/
		unsigned short HostPort() const;

		/**
		*  获取地址是否为空,
		* @return 返回true表示地址为空
		*/
		bool Empty() const;

		/**
		*  获取地址的文字表示方式
		* @return 地址的文字表示方式,如192.160.0.100:10001
		*/
		const std::string& ToString() const;

		bool operator == (const EndPoint &right) const;

		bool operator != (const EndPoint &right) const;

		bool operator < (const EndPoint &right) const;

		bool operator <= (const EndPoint &right) const;

		/**
		*  获取套接字对应的本地地址
		* @param socket 套接字
		* @return 本地地址
		*/
		static EndPoint GetLocalEndPoint(int socket);

		/**
		*  获取套接字对应的远程地址
		* @param socket 套接字
		* @return 远程地址
		*/
		static EndPoint GetRemoteEndPoint(int socket);

#ifndef _WIN32
		/**
		*  获取网卡ip
		* @param deviceName 网卡名
		* @return 网卡ip
		*/
		static std::string GetIp(const std::string& deviceName);

		/**
		*  设置网卡ip
		* @param deviceName 网卡名
		* @param ip ip
		* @return 操作结果
		*/
		static bool SetIp(const std::string& deviceName, const std::string& ip);

		/**
		*  获取子网掩码
		* @param deviceName 网卡名
		* @return 子网掩码
		*/
		static std::string GetMask(const std::string& deviceName);

		/**
		*  设置子网掩码
		* @param deviceName 网卡名
		* @param mask 子网掩码
		* @return 操作结果
		*/
		static bool SetMask(const std::string& deviceName, const std::string& mask);

		/**
		*  获取网关地址
		* @param deviceName 网卡名
		* @return 网关地址
		*/
		static std::string GetGateway(const std::string& deviceName);

		/**
		*  设置网关地址
		* @param deviceName 网卡名
		* @param mask 网关地址
		* @return 操作结果
		*/
		static bool SetGateway(const std::string& deviceName, const std::string& gateway);
		
		/**
		*  获取MAC地址
		* @param deviceName 网卡名
		* @return MAC地址
		*/
		static std::string GetMac(const std::string& deviceName);

#endif // !_WIN32

	private:
		/**
		* 初始化地址字符串
		* @param ip ip
		* @param port 端口的网络端表示方式
		*/
		void InitAddress(unsigned int ip, unsigned short port);

		// ip的数字表示方式
		unsigned int _netIp;

		// 端口的网络端表示方式 
		unsigned short _netPort;

		// 端口的主机端表示方式 
		unsigned short _hostPort;

		//ip字符串表示方式
		std::string _hostIp;

		//地址字符串表示方式
		std::string _address;

		//计算机名最大长度
		const static int HOSTNAMEMAX = 256;
	};
}


