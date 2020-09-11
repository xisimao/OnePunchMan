#pragma once
#ifdef _WIN32 
#include <WinSock2.h>
#include <mstcpip.h>  
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#endif

#include "EndPoint.h"
#include "LogPool.h"

namespace OnePunchMan
{

#define StatusPort 7771

	//套接字类型
	enum class SocketType
	{
		None,
		//Tcp监听
		Listen,
		//Tcp客户端
		Accept,
		//Tcp服务端
		Connect,
		//Udp
		Udp
	};

	//套接字操作
	enum class SocketOperation
	{
		Add,
		Remove,
		Assign,
		Error
	};

	//发送结果
	enum class SocketResult
	{
		Success = 0,
		SendFailed = 1,
		Timeout = 2,
		Disconnection = 3,
		NotFoundSocket = 4,
		InvalidTag = 5
	};

	//套接字数据
	class SocketData
	{
	public:

		//套接字
		int Socket;
		//套接字开始的时间
		DateTime StartTime;
		//套接字类型
		SocketType Type;
		//监听端口
		EndPoint Remote;
		//本地端口
		EndPoint Local;
		//套接字标记
		unsigned short Tag;

		SocketData()
			:Socket(0),Type(SocketType::None),Tag(0)
		{

		}
	};

	//套接字操作
	class Socket
	{
	public:

		/**
		* @brief: 启动windows socket
		* @return: bool 启动成功返回true
		*/
		static bool Init();

		/**
		* @brief: 清理windows socket
		*/
		static void Uninit();

		/**
		* @brief: 创建一个udp套接字
		* @return: udp套接字
		*/
		static int UdpSocket();

		/**
		* @brief: 绑定本地地址,用作Udp多播服务
		* @param: endPoint 本地地址
		* @return: 返回0表示监听时出现错误,否则返回绑定套接字
		*/
		static int MultiCast(const EndPoint& endPoint);

		/**
		* @brief: 绑定本地地址,用作Udp服务
		* @param: endPoint 本地地址
		* @return: 返回0表示监听时出现错误,否则返回绑定套接字
		*/
		static int Bind(const EndPoint& endPoint);

		/**
		* @brief: 监听本地端口,用作Tcp服务
		* @param: endPoint 本地地址
		* @return: 返回0表示监听时出现错误,否则返回监听套接字
		*/
		static int Listen(const EndPoint& endPoint);

		/**
		* @brief: 连接到Tcp服务
		* @param: endPoint 远程地址
		* @return: 返回0表示连接失败,否则返回套接字
		*/
		static int ConnectTcp(const EndPoint& endPoint);

		/**
		* @brief: 发送字节流
		* @param: socket 套接字
		* @param: buffer 字节流缓冲
		* @param: size 字节流长度
		* @return: 返回true表示发送成功
		*/
		static bool SendTcp(int socket, const char* buffer, unsigned int size);

		/**
		* @brief: 发送字节流
		* @param: socket 套接字
		* @param: endPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: size 字节流长度
		* @return: 返回true表示发送成功
		*/
		static bool SendUdp(int socket, const EndPoint& endPoint, const char* buffer, unsigned int size);

		/**
		* @brief: 关闭套接字
		* @param: socket 套接字
		*/
		static void Close(int socket);
	};
}