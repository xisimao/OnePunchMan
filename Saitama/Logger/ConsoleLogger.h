#pragma once
#include <string>
#include <iostream>

#include "Logger.h"

namespace Saitama
{
	//控制台日志类
	class ConsoleLogger : public Logger
	{
	public:
		/**
		* @brief: 构造函数
		* @param: minLevel 日志最小级别
		* @param: maxLevel 日志最大级别
		*/
		ConsoleLogger(LogLevel minLevel, LogLevel maxLevel);

	protected:
		
		void LogCore(const std::string& log);

	};
}

