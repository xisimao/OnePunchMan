#pragma once
#include <thread>
#include <string>

#include "LogPool.h"

namespace OnePunchMan
{
	//线程对象基类
	class ThreadObject
	{
	public:

		/**
		* 构造函数
		*/
		ThreadObject(const std::string& name);

		/**
		* 析构函数
		*/
		virtual ~ThreadObject();

		/**
		* 获取线程名称
		* @return 线程名称
		*/
		std::string Name() const;

		/**
		* 开始线程
		*/
		void Start();

		/**
		* 等待线程结束
		*/
		void Join();

		/**
		* 向线程提交取消请求,并阻塞调用线程直到线程终止为止。
		*/
		void Stop();

		//轮询中睡眠时间(秒)
		static const int LockTime;

	protected:

		/**
		* 子类需要实现的具体启动程序
		* @param arg 启动参数
		*/
		virtual void StartCore() = 0;

		/**
		* 子类选择实现的停止启动程序
		* @param arg 启动参数
		*/
		virtual void StopCore()
		{
		}

		// 当前程序是否已经提交了取消请求
		bool _cancelled;

	private:

		// 线程状态
		enum class ThreadStatus
		{
			Unstarted = 0,
			Running = 1,
			Stopped = 2
		};

		//轮询中睡眠时间(毫秒)
		static const int SleepTime;

		/**
		* 线程启动函数
		* @param arg 启动参数
		*/
		void StartThread();

		//线程名称
		std::string _name;

		// 线程状态
		ThreadStatus _status;

		// 线程 
		std::thread _thread;

	};

}


