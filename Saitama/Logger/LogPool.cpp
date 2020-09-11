#include "LogPool.h"

using namespace std;
using namespace OnePunchMan;

LogPool* LogPool::_instance= NULL;

LogPool::LogPool(const JsonDeserialization& jd)
{
	//读取配置文件
	_directory = jd.Get("Logging:Directory", string("logs"));
	_holdDays = jd.Get<unsigned int>("Logging:HoldDays");
	_defaultLevel = ReadLevel(jd,"Logging:Default");
	//添加文件日志
	unsigned int index = 0;
	while (true)
	{
		string loggerTag = StringEx::Combine("Logging:Logger:", index);
		LogType type = ReadType(jd, loggerTag + ":Type");
		if (type == LogType::None)
		{
			break;
		}
		LogLevel minLevel = ReadLevel(jd, loggerTag + ":MinLevel");
		LogLevel maxLevel = ReadLevel(jd, loggerTag + ":MaxLevel");
		if (type == LogType::Console)
		{
			_loggers.insert(new ConsoleLogger(minLevel, maxLevel));
		}
		else if (type == LogType::File)
		{
			string name = jd.Get<string>(loggerTag + ":Name");
			if (!name.empty())
			{
				_loggers.insert(new FileLogger(minLevel, maxLevel, name, _directory, _holdDays));
			}
		}
		index += 1;
	}
}

LogPool::~LogPool()
{
	for_each(_loggers.begin(), _loggers.end(), [](Logger* logger) {
		delete logger;
	});
}

void LogPool::Init(const JsonDeserialization& jd)
{
	_instance = new LogPool(jd);
}

void LogPool::AddLogger(Logger* logger)
{
	std::lock_guard<std::mutex> lck(_instance->_mutex);
	_instance->_loggers.insert(logger);
}

string LogPool::Directory()
{
	if (_instance == NULL)
	{
		return string();
	}
	else
	{
		return _instance->_directory;
	}
}

int LogPool::HoldDays()
{
	if (_instance == NULL)
	{
		return 0;
	}
	else
	{
		return _instance->_holdDays;
	}
}

LogType LogPool::ReadType(const JsonDeserialization& jd, const string& key)
{
	string value = jd.Get<string>(key);
	if (!value.empty())
	{
		value = StringEx::ToUpper(value);
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

LogLevel LogPool::ReadLevel(const JsonDeserialization& jd, const string& key)
{
	string value = jd.Get<string>(key);
	if (!value.empty())
	{
		value = StringEx::ToUpper(value);
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
