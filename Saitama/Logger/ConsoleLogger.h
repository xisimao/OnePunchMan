#pragma once
#include <string>
#include <iostream>

#include "Logger.h"

namespace OnePunchMan
{
	//控制台日志类
	class ConsoleLogger : public Logger
	{
	public:
		/**
		* 构造函数
		* @param minLevel 日志最小级别
		* @param maxLevel 日志最大级别
		*/
		ConsoleLogger(LogLevel minLevel, LogLevel maxLevel);

	protected:	
		void LogCore(LogLevel logLevel, LogEvent logEvent, const DateTime& time, const std::string& content);

	};
}

