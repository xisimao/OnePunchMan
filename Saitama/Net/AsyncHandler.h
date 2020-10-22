#pragma once
#include <mutex>
#include <condition_variable>

#include "DateTime.h"

namespace OnePunchMan
{
	//socket异步调用
	class AsyncHandler
	{
	public:

		/**
		* 构造函数
		* @param timeStamp 发送时间戳
		* @param protocolId 等待响应的协议编号
		*/
		AsyncHandler(long long timeStamp,unsigned int protocolId);

		/**
		* 析构函数
		*/
		virtual ~AsyncHandler()
		{

		}

		/**
		* 返回需要监听的编号
		* @return 需要监听的编号
		*/
		unsigned int ProtocolId();

		/**
		* 返回发送的时间戳
		* @return 发送的时间戳
		*/
		long long TimeStamp();

		/**
		* 表示是否执行完成
		* @return 返回true表示已经执行完成
		*/
		virtual bool IsCompleted() = 0;

		/**
		* 执行字节流
		* @param begin 字节流开始
		* @param end 字节流结束
		*/
		virtual void Handle(std::string::const_iterator begin, std::string::const_iterator end) = 0;

	private:

		//发送时间戳
		long long _timeStamp;

		//需要监听的编号
		unsigned int _protocolId;
	};
}