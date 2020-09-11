#pragma once
#include <string.h> //linux memcpy

#include "AsyncHandler.h"

namespace OnePunchMan
{
	//通知协议到达
	class NoticeHandler : public AsyncHandler
	{
	public:

		/**
		* @brief: 构造函数
		* @param: protocolId 等待响应的协议编号
		*/
		NoticeHandler(unsigned int protocolId);
		/**
		* @brief: 构造函数
		* @param: protocolId 等待响应的协议编号
		* @param: buffer 用于获取返回字节流的指针
		*/
		NoticeHandler(unsigned int protocolId, std::string* buffer);

		/**
		* @brief: 构造函数
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		*/
		NoticeHandler(long long timeStamp, unsigned int protocolId);

		/**
		* @brief: 构造函数
		* @param: timeStamp 发送时间戳
		* @param: protocolId 等待响应的协议编号
		* @param: buffer 用于获取返回字节流的指针
		*/
		NoticeHandler(long long timeStamp, unsigned int protocolId, std::string* buffer);

		/**
		* @brief: 等待通知
		* @return: 返回true表示等待到通知,返回false表示超时
		*/
		bool Wait(int milliseconds=3500);

		bool IsCompleted();

		void Handle(std::string::const_iterator begin, std::string::const_iterator end);

	private:

		/**
		* @brief: 执行结束后通知
		*/
		void Noticty();

		//表示是否已经执行完成
		bool _isCompleted;

		//同步锁
		std::mutex _mutex;

		//条件变量
		std::condition_variable _condition;

		//用于接收返回数据的字节流
		std::string* _buffer;

	};

}
