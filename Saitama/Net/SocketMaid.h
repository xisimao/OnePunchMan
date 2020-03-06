#pragma once
#include "Observable.h"
#include "SocketChannel.h"
#include "EndPointChannel.h"
#include "NoticeHandler.h"

namespace Saitama
{

	//协议通信
	class SocketMaid :
		public ThreadObject,
		public IObserver<AcceptedEventArgs>,
		public IObserver<ClosedEventArgs>,
		public IObserver<ConnectedEventArgs>
	{
	public:

		/**
		* @brief: 构造函数
		*/
		SocketMaid();

		/**
		* @brief: 构造函数
		* @param: channelCount 通道数量
		*/
		SocketMaid(unsigned int channnelCount);

		/**
		* @brief: 析构函数
		*/
		~SocketMaid();

		/**
		* @brief: 添加监听地址
		* @param: endPoint 监听地址
		* @param: handler 套接字操作指针
		* @return: 返回-1表示监听失败，否则返回套接字
		*/
		int AddListenEndPoint(const EndPoint& endPoint, SocketHandler* handler);

		/**
		* @brief: 添加tcp地址
		* @param: endPoint 连接地址
		* @param: handler 套接字操作指针
		*/
		void AddConnectEndPoint(const EndPoint& endPoint, SocketHandler* handler);

		/**
		* @brief: 添加Udp套接字
		* @param: handler 套接字操作指针
		* @param: channelIndex 通道编号，-1表示自动适配
		* @return: 返回-1表示绑定失败，否则返回套接字
		*/
		int AddUdpSocket(SocketHandler* handler, int channelIndex = -1);

		/**
		* @brief: 添加Udp地址
		* @param: endPoint Udp地址
		* @param: handler 套接字操作指针
		* @param: channelIndex 通道编号，-1表示自动适配
		* @return: 返回-1表示绑定失败，否则返回套接字
		*/
		int AddBindEndPoint(const EndPoint& endPoint, SocketHandler* handler, int channelIndex = -1);

		/**
		* @brief: 添加Udp多播地址
		* @param: endPoint Udp地址
		* @param: handler 套接字操作指针
		* @param: channelIndex 通道编号，-1表示自动适配
		* @return: 返回-1表示绑定失败，否则返回套接字
		*/
		int AddMultiCastEndPoint(const EndPoint& endPoint, SocketHandler* handler, int channelIndex = -1);

		/**
		* @brief: 移除连接地址
		* @param: endPoint 连接地址
		*/
		void RemoveEndPoint(const EndPoint& endPoint);

		/**
		* @brief: 移除套接字
		* @param: socket 套接字
		* @param: result 移除原因
		*/
		void RemoveSocket(int socket,int result=0);

		/**
		* @brief: 发送字节流
		* @param: socket 套接字
		* @param: buffer 字节流缓冲
		* @return: 发送结果
		*/
		SocketResult SendTcp(int socket, const std::string& buffer);

		/**
		* @brief: 发送字节流
		* @param: endPoint 地址
		* @param: buffer 字节流缓冲
		* @return: 发送结果
		*/
		SocketResult SendTcp(const EndPoint& endPoint, const std::string& buffer);

		/**
		* @brief: 发送字节流
		* @param: tag 客户端标记
		* @param: buffer 字节流缓冲
		* @return: 发送结果
		*/
		SocketResult SendTcp(unsigned short tag, const std::string& buffer);

		/**
		* @brief: 发送字节流，并等待响应，但是不关心返回结果
		* @param: socket 套接字
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @return: 发送结果
		*/
		SocketResult SendTcp(int socket, const std::string& buffer, long long timeStamp,unsigned int protocolId);

		/**
		* @brief: 发送字节流，并等待响应，但是不关心返回结果
		* @param: endPoint 地址
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @return: 发送结果
		*/
		SocketResult SendTcp(const EndPoint& endPoint, const std::string& buffer, long long timeStamp, unsigned int protocolId);

		/**
		* @brief: 发送字节流，并等待响应，但是不关心返回结果
		* @param: tag 客户端标记
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @return: 发送结果
		*/
		SocketResult SendTcp(unsigned short tag, const std::string& buffer, long long timeStamp, unsigned int protocolId);

		/**
		* @brief: 发送字节流，并等待响应，需要获取返回结果
		* @param: socket 套接字
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @param: responseBuffer 用于获取返回字节流的指针
		* @return: 发送结果
		*/
		SocketResult SendTcp(int socket, const std::string& buffer, long long timeStamp, unsigned int protocolId, std::string* responseBuffer);

		/**
		* @brief: 发送字节流，并等待响应，需要获取返回结果
		* @param: endPoint 地址
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @param: responseBuffer 用于获取返回字节流的指针
		* @return: 发送结果
		*/
		SocketResult SendTcp(const EndPoint& endPoint, const std::string& buffer, long long timeStamp, unsigned int protocolId, std::string* responseBuffer);

		/**
		* @brief: 发送字节流，并等待响应，需要获取返回结果
		* @param: tag 客户端标记
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @param: responseBuffer 用于获取返回字节流的指针
		* @return: 发送结果
		*/
		SocketResult SendTcp(unsigned short tag, const std::string& buffer, long long timeStamp, unsigned int protocolId, std::string* responseBuffer);

		/**
		* @brief: 发送字节流
		* @param: socket 套接字
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @return: 发送结果
		*/
		SocketResult SendUdp(int socket, const EndPoint& remoteEndPoint, const std::string& buffer);

		/**
		* @brief: 发送字节流
		* @param: bindEndPoint 绑定地址
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @return: 发送结果
		*/
		SocketResult SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const std::string& buffer);

		/**
		* @brief: 发送字节流，并等待响应，但是不关心返回结果
		* @param: socket 套接字
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @return: 发送结果
		*/
		SocketResult SendUdp(int socket, const EndPoint& remoteEndPoint, const std::string& buffer, long long timeStamp, unsigned int protocolId);

		/**
		* @brief: 发送字节流，并等待响应，但是不关心返回结果
		* @param: bindEndPoint 绑定地址
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @return: 发送结果
		*/
		SocketResult SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const std::string& buffer, long long timeStamp, unsigned int protocolId);

		/**
		* @brief: 发送字节流，并等待响应，需要获取返回结果
		* @param: socket 套接字
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @param: responseBuffer 用于获取返回字节流的指针
		* @return: 发送结果
		*/
		SocketResult SendUdp(int socket, const EndPoint& remoteEndPoint, const std::string& buffer, long long timeStamp, unsigned int protocolId,std::string* responseBuffer);

		/**
		* @brief: 发送字节流，并等待响应，需要获取返回结果
		* @param: bindEndPoint 绑定地址
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @param: responseBuffer 用于获取返回字节流的指针
		* @return: 发送结果
		*/
		SocketResult SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const std::string& buffer, long long timeStamp, unsigned int protocolId, std::string* responseBuffer);

		//套接字事件函数
		virtual void Update(AcceptedEventArgs* e);

		virtual void Update(ConnectedEventArgs* e);

		virtual void Update(ClosedEventArgs* e);
		
	protected:

		/**
		* @brief: 添加Udp地址
		* @param: endPoint Udp地址
		* @param: socket Udp套接字
		* @param: handler 套接字操作指针
		* @param: channelIndex 通道编号，-1表示自动适配
		*/
		void AddUdpEndPoint(const EndPoint& endPoint,int socket, SocketHandler* handler, int channelIndex = -1);

		/**
		* @brief: 发送字节流，实现异步通知功能
		* @param: socket 套接字
		* @param: buffer 字节流缓冲
		* @param: handler 异步接口
		* @return: 发送结果
		*/
		SocketResult SendTcp(int socket, const std::string& buffer, AsyncHandler* handler);

		/**
		* @brief: 发送字节流，实现异步通知功能
		* @param: endPoint 地址
		* @param: buffer 字节流缓冲
		* @param: handler 异步接口
		* @return: 发送结果
		*/
		SocketResult SendTcp(const EndPoint& endPoint, const std::string& buffer, AsyncHandler* handler);

		/**
		* @brief: 发送字节流，实现异步通知功能
		* @param: tag 客户端标记
		* @param: buffer 字节流缓冲
		* @param: handler 异步接口
		* @return: 发送结果
		*/
		SocketResult SendTcp(unsigned short tag, const std::string& buffer, AsyncHandler* handler);

		/**
		* @brief: 发送字节流，实现异步通知功能
		* @param: socket 套接字
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: handler 异步接口
		* @return: 发送结果
		*/
		SocketResult SendUdp(int socket, const EndPoint& remoteEndPoint, const std::string& buffer, AsyncHandler* handler);

		/**
		* @brief: 发送字节流，实现异步通知功能
		* @param: bindEndPoint 绑定地址
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: handler 异步接口
		* @return: 发送结果
		*/
		SocketResult SendUdp(const EndPoint& bindEndPoint, const EndPoint& remoteEndPoint, const std::string& buffer, AsyncHandler* handler);

		void StartCore();

		/**
		* @brief: 供子类实现，线程开始的操作
		*/
		virtual void InitCore()
		{

		}

		/**
		* @brief: 供子类实现，在线程中每个轮询执行的操作
		*/	
		virtual void PollCore()
		{

		}

		/**
		* @brief: 供子类实现，线程结束的操作
		*/
		virtual void ExitCore()
		{

		}

		/**
		* @brief: 设置客户端套接字的标记
		* @param: socket 客户端套接字
		* @param: tag 客户端标记
		*/
		void SetTag(int socket, unsigned short tag);

		/**
		* @brief: 设置套接字日志
		* @param: socket 套接字
		* @param: logName 日志名
		*/
		void SetLogger(int socket, const std::string& logName);

		/**
		* @brief: 选择通道
		* @return: 返回Null表示没有可用通道，否则返回通道指针
		*/
		virtual SocketChannel* Select();

		//套接字数据
		class SocketItem
		{
		public:
			//套接字所在通道
			SocketChannel* Channel;
			//套接字操作指针
			SocketHandler* Handler;
			//套接字操作
			SocketOperation Operation;
			//套接字数据
			SocketData Data;
		};

		//套接字通道集合
		std::vector<SocketChannel*> _channels;

		//套接字数据字典
		std::map<int, SocketItem> _sockets;

		//套接字集合同步锁
		std::mutex _socketMutex;

		//线程集合
		std::set<ThreadObject*> _threads;

		//线程集合
		std::mutex _threadMutex;

		//线程轮询序号
		unsigned int _pollIndex;

	private:

		/**
		* @brief: 获取客户端标记的套接字
		* @param: tag 客户端标记
		* @return: 未找到返回0
		*/
		int GetSocket(unsigned short tag);

		//当前选择的通道序号
		unsigned int _channelIndex;

		//套接字连接
		EndPointChannel _connection;

		//tpc客户端标记
		std::map<unsigned short, int> _tags;
	};
}


