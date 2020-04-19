#pragma once
#include <thread>
#include <string>
#include <set>

#include "LogPool.h"

namespace Saitama
{
	//线程对象基类
	class ThreadObject
	{
	public:

		/**
		* @brief: 构造函数
		*/
		ThreadObject(const std::string& name);

		/**
		* @brief: 析构函数
		*/
		virtual ~ThreadObject();

		/**
		* @brief: 获取线程名称
		* @return: 线程名称
		*/
		std::string Name() const;

		/**
		* @brief: 开始线程
		*/
		void Start();

		/**
		* @brief: 等待线程结束
		*/
		void Join();

		/**
		* @brief: 向线程提交取消请求，并阻塞调用线程直到线程终止为止。
		*/
		void Stop();

	protected:

		/**
		* @brief: 子类需要实现的具体启动程序
		* @param: arg 启动参数
		*/
		virtual void StartCore() = 0;

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
		* @brief: 线程启动函数
		* @param: arg 启动参数
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


