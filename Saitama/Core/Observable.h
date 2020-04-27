#pragma once
#include <set>
#include <mutex>
#include <algorithm>

namespace OnePunchMan
{
	// 提供用于接收基于推送的通知的机制
	template <typename T>
	class IObserver
	{
	public:

		/**
		* @brief: 析构函数
		*/
		virtual ~IObserver()
		{

		}

		/**
		* @brief: 向观察者提供新数据
		* @param: 当前的通知信息
		*/
		virtual void Update(T* t) = 0;
	};

	// 定义基于推送的通知的提供程序
	template <typename T>
	class Observable
	{
	public:

		/**
		* @brief: 析构函数
		*/
		virtual ~Observable(void)
		{

		}

		/**
		* @brief: 通知提供程序：某观察程序将要接收通知。
		* @param: observer 观察者
		*/
		void Subscribe(IObserver<T>* observer)
		{
			std::lock_guard<std::mutex> lck(_mutex);
			_observers.insert(observer);
		}

		/**
		* @brief: 通知提供程序：某观察程序将要取消通知。
		* @param: observer 观察者
		*/
		void Unsubscribe(IObserver<T>*  observer)
		{
			std::lock_guard<std::mutex> lck(_mutex);
			_observers.erase(observer);
		}
		
		/**
		* @brief: 向所有观察者提供新数据
		* @param: t 当前的通知信息。
		*/
		void Notice(T* t)
		{
			std::unique_lock<std::mutex> lck(_mutex);
			std::set<IObserver<T>*> temp(_observers);
			lck.unlock();
			for_each(temp.begin(), temp.end(),[&t](IObserver<T>* o){
				o->Update(t);
			});
		}

		/**
		* @brief: 复制另外一个通知程序的观察者集合
		* @param: o 另外一个通知程序
		*/
		void Copy(Observable* o)
		{
			this->_observers = o->_observers;
		}

	private:

		// 观察者集合
		std::set<IObserver<T>* > _observers;

		//同步锁
		std::mutex _mutex;
	};
}
