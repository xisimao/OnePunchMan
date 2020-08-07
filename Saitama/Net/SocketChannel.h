#pragma once
#ifdef _WIN32 
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#endif

#include <map>
#include <mutex>
#include <vector>
#include <algorithm>

#include "Observable.h"
#include "EndPoint.h"
#include "Socket.h"
#include "SocketHandler.h"
#include "Thread.h"
#include "LogPool.h"

namespace OnePunchMan
{

	//tcp客户端连接事件
	class AcceptedEventArgs
	{
	public:
		//监听套接字
		int ListenSocket;
		//客户端套接字
		int TcpSocket;
		//套接字操作指针
		SocketHandler* Handler;
	};

	//tcp套接字接收出错或关闭事件参数
	class ClosedEventArgs
	{
	public:
		//关闭的套接字
		int Socket;
		//rece的返回值
		int Result;

		ClosedEventArgs()
			:Socket(0),Result(0)
		{

		}
	};

	//socket通道
	class SocketChannel : public ThreadObject
	{
	public:

		/**
		* @brief: 构造函数
		*/
		SocketChannel();

		/**
		* @brief: 析构函数
		*/
		virtual ~SocketChannel()
		{
#ifndef _WIN32 
			Socket::Close(_epollfd);
#endif
		}

		/**
		* @brief: 添加监听套接字
		* @param: socket 套接字
		* @param: type 套接字类型
		* @param: handler 套接字操作指针
		*/
		void AddSocket(int socket,SocketType type, SocketHandler* handler);

		/**
		* @brief: 被动移除套接字
		* @param: socket 套接字
		*/
		void RemoveSocket(int socket);

		//客户端连接事件
		Observable<AcceptedEventArgs> Accepted;

		//套接字关闭事件
		Observable<ClosedEventArgs> Closed;

	protected:

		void StartCore();

	private:

		//套接字数据
		class SocketItem
		{
		public:
			//套接字
			int Socket;
			//套接字类型
			SocketType Type;
			//套接字操作
			SocketOperation Operation;
			//套接字执行指针
			SocketHandler* Handler;

			SocketItem()
				:Socket(0), Type(SocketType::None),Operation(SocketOperation::Error),Handler(NULL)
			{

			}

			SocketItem(int socket,SocketType type, SocketHandler* handler, SocketOperation operation)
			{
				Socket = socket;
				Type = type;
				Handler = handler;
				Operation = operation;
			}
		};

		/**
		* @brief: 移动临时套接字集合
		*/
		void MoveTempSockets();

		//轮询超时时间
		static const int Timeout;

		//字节流缓冲长度
		static const int BufferLength;

		//准备好在下次轮询中添加或移除的套接字
		std::vector<SocketItem> _tempSockets;
		//同步锁
		std::mutex _mutex;

		//当前监听的套接字字典
		std::map<int, SocketItem> _sockets;

#ifndef _WIN32 
		// epoll套接字
		int _epollfd;

		//epoll最大事件数
		static const int MaxEvents;
#endif

	};

}

