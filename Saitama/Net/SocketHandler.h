#pragma once
#include <mutex>
#include <list>
#include <tuple>

#include "Observable.h"
#include "Socket.h"
#include "AsyncHandler.h"

namespace Saitama
{
	//套接字解析和执行
	class SocketHandler
	{
	public:

		/**
		* @brief: 构造函数,不使用日志
		*/
		SocketHandler();

		/**
		* @brief: 虚构函数
		*/
		virtual ~SocketHandler()
		{
		}

		/**
		* @brief: 发送字节流
		* @param: socket 套接字
		* @param: buffer 字节流缓冲
		* @param: handler 异步操作指针，不使用传入NULL
		* @return: 发送结果
		*/
		SocketResult SendTcp(int socket, const std::string& buffer, AsyncHandler* handler);

		/**
		* @brief: 发送字节流
		* @param: socket 套接字
		* @param: remoteEndPoint 远程地址
		* @param: buffer 字节流缓冲
		* @param: handler 异步操作指针，不使用传入NULL
		* @return: 发送结果
		*/
		SocketResult SendUdp(int socket, const EndPoint& remoteEndPoint, const std::string& buffer, AsyncHandler* handler);

		/**
		* @brief: 解析字节流
		* @param: socket 套接字
		* @param: ip 发送端ip
		* @param: port 发送端端口
		* @param: buffer 字节流
		* @param: size 字节流长度
		*/
		void Handle(int socket,unsigned int ip, unsigned short port, const char* buffer, unsigned int size);

		/**
		* @brief: 返回当前套接字发送的总流量
		* @return: 当前套接字发送的总流量
		*/
		unsigned long long TransmitSize();

		/**
		* @brief: 返回当前套接字接收的总流量
		* @return: 当前套接字接收的总流量
		*/
		unsigned long long ReceiveSize();

		/**
		* @brief: 创建一个全新的实例
		* @return: 新实例指针
		*/
		SocketHandler* Clone(int socket);

	protected:

		//分析结果
		enum class AnalysisResult
		{
			//未分析出协议
			Empty,
			//半包
			Half,
			//请求协议
			Request,
			//响应协议
			Response
		};

		class ProtocolPacket
		{
		public:
			AnalysisResult Result;
			unsigned int Offset;
			unsigned int Size;
			unsigned int ProtocolId;
			long long TimeStamp;

			ProtocolPacket()
				:ProtocolPacket(AnalysisResult::Empty,0,0,0,0)
			{

			}

			ProtocolPacket(AnalysisResult result, unsigned int offset, unsigned int size, unsigned int protocolId, long long timeStamp)
				:Result(result),Offset(offset),Size(size),ProtocolId(protocolId),TimeStamp(timeStamp)
			{

			}

		};

		/**
		* @brief: 供子类实现的创建实例方法
		* @return: 新实例指针
		*/
		virtual SocketHandler* CloneCore() { return new SocketHandler(); };

		/**
		* @brief: 执行字节流
		* @param: socket 套接字
		* @param: ip 发送端ip
		* @param: port 发送端端口
		* @param: begin 字节流开始
		* @param: end 字节流结尾
		* @return: 协议包
		*/
		virtual ProtocolPacket HandleCore(int socket, unsigned int ip, unsigned short port, std::string::const_iterator begin, std::string::const_iterator end)
		{
			ProtocolPacket packet;
			packet.Offset = 0;
			packet.Size = static_cast<unsigned int>(end - begin);
			packet.Result = AnalysisResult::Response;
			packet.ProtocolId = 0;
			packet.TimeStamp = 0;
			return packet;
		}

	private:

		//半包临时存放
		std::string _residueBuffer;

		//发送总流量
		unsigned long long _transmitSize;

		//接收总流量
		unsigned long long _receiveSize;

		//异步操作指针集合
		std::list<AsyncHandler*> _handlers;

		//同步锁
		std::mutex _mutex;
	};
}