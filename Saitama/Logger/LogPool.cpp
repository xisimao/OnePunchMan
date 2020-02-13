#include "LogPool.h"

using namespace std;
using namespace Saitama;

LogPool LogPool::_instance;

LogPool::LogPool()
{
	//读取文件日志参数
	FileConfig config;

	//添加文件日志
	unsigned int index = 1;
	stringstream ss;
	ss << "Logger." << index;
	LogType type;
	while ((type=ReadType(config,ss.str()+".Type"))!=LogType::None)
	{
		LogLevel minLevel = ReadLevel(config, ss.str() + ".MinLevel");
		LogLevel maxLevel = ReadLevel(config, ss.str() + ".MaxLevel");
		if (type == LogType::Console)
		{
			_loggers.insert(new ConsoleLogger(minLevel, maxLevel));
		}
		else if (type == LogType::File)
		{
			string name = ReadName(config, ss.str() + ".Name");
			if (!name.empty())
			{
				_loggers.insert(new FileLogger(name,minLevel, maxLevel));
			}
		}

		++index;
		ss.str("");
		ss << "Logger." << index;
	}
}

LogPool::~LogPool()
{
	for_each(_loggers.begin(), _loggers.end(), [](Logger* logger) {
	
		delete logger;
	});
}

void LogPool::AddLogger(Logger* logger)
{
	lock_guard<mutex> lck(_instance._mutex);
	_instance._loggers.insert(logger);
}

void LogPool::RemoveLogger(Logger* logger)
{
	lock_guard<mutex> lck(_instance._mutex);
	_instance._loggers.erase(logger);
}

LogType LogPool::ReadType(const FileConfig& config, const string& key)
{
	string value;
	if (config.ReadConfig<string>(key, &value))
	{
		value = Text::ToUpper(value);
		if (value.compare("CONSOLE") == 0)
		{
			return LogType::Console;
		}
		else if (value.compare("FILE") == 0)
		{
			return LogType::File;
		}
	}
	return LogType::None;
}

string LogPool::ReadName(const FileConfig& config, const string& key)
{
	string value;
	config.ReadConfig<string>(key, &value);
	return value;
}

LogLevel LogPool::ReadLevel(const FileConfig& config, const string& key)
{
	string value;
	if (config.ReadConfig<string>(key, &value))
	{
		value = Text::ToUpper(value);
		if (value.compare("DEBUG") == 0)
		{
			return LogLevel::Debug;
		}
		else if (value.compare("INFORMATION") == 0)
		{
			return LogLevel::Information;
		}
		else if (value.compare("WARNING") == 0)
		{
			return LogLevel::Warning;
		}
		else if (value.compare("ERROR") == 0)
		{
			return LogLevel::Error;
		}
	}
	return LogLevel::None;
}
