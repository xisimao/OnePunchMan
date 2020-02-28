#pragma once
#include <map>
#include <set>
#include <mutex>
#include <condition_variable>

#include "Observable.h"
#include "Socket.h"
#include "SocketHandler.h"
#include "Thread.h"

namespace Saitama
{
	// tcp套接字连接事件参数
	class ConnectedEventArgs
	{
	public:
		//连接上的地址
		EndPoint Ep;
		//连接套接字
		int Socket;
		//套接字操作指针
		SocketHandler* Handler;
	};

	//连接轮询类
	class EndPointChannel : public ThreadObject
	{
	public:

		/**
		* @brief: 构造函数
		*/
		EndPointChannel();

		//连接到服务事件
		Observable<ConnectedEventArgs> Connected;

		/**
		* @brief: 添加地址
		* @param: endPoint 地址
		* @param: handler 操作指针
		*/
		void AddTcpEndPoint(const EndPoint& endPoint,SocketHandler* handler);

		/**
		* @brief: 添加地址和套接字
		* @param: endPoint 地址
		*/
		void AddUdpEndPoint(const EndPoint& endPoint, int socket);

		/**
		* @brief: 移除地址
		* @param: endPoint 地址
		*/
		void RemoveEndPoint(const EndPoint& endPoint);

		/**
		* @brief: 报告错误套接字
		* @param: socket 套接字
		*/
		void ReportTcpError(int socket);

		/**
		* @brief: 获取终结点的套接字
		* @param: endPoint 地址
		* @return: 返回0表示未找到地址，返回-1表示未连接，否则返回套接字
		*/
		int GetSocket(const EndPoint& endPoint);

	protected:

		void StartCore();

		void StopCore();

	private:

		//地址数据
		class EndPointItem
		{
		public:
			int Socket;
			EndPoint Ep;
			SocketType Type;
			SocketHandler* Handler;
			SocketOperation Operation;

			EndPointItem()
			{

			}

			EndPointItem(int socket,const EndPoint& endPoint, SocketType type, SocketHandler* handler, SocketOperation operation)
				:Socket(socket),Ep(endPoint),Type(type),Handler(handler),Operation(operation)
			{

			}
		};

		/**
		* @brief: 移动临时地址集合
		*/
		void MoveTempEndPoints();

		//重连轮询时间间隔
		static const int ConnectionSpan;

		//临时添加和删除地址字典
		std::vector<EndPointItem> _tempEndPoints;
		//同步锁
		std::mutex _mutex;

		//客户端套接字
		std::map<EndPoint, EndPointItem> _endPoints;

		//条件变量
		std::condition_variable _condition;
	};

}
