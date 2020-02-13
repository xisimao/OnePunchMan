#pragma once

namespace Saitama
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
}
