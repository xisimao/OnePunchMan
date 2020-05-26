#include "LogPool.h"

using namespace std;
using namespace OnePunchMan;

LogPool* LogPool::_instance=NULL;

LogPool::LogPool(const string& filePath)
{
	//读取文件日志参数
	FileConfig config(filePath);

	//读取配置文件
	_directory = config.Get("Logging:Directory", string("logs"));
	_holdDays = config.Get<unsigned int>("Logging:HoldDays");
	_defaultLevel = ReadLevel(config, "Logging:Default");
	//添加文件日志
	unsigned int index = 0;
	while (true)
	{
		string loggerTag = StringEx::Combine("Logging:Logger:", index);
		LogType type = ReadType(config, loggerTag + ":Type");
		if (type == LogType::None)
		{
			break;
		}
		LogLevel minLevel = ReadLevel(config, loggerTag + ":MinLevel");
		LogLevel maxLevel = ReadLevel(config, loggerTag + ":MaxLevel");
		if (type == LogType::Console)
		{
			_loggers.insert(new ConsoleLogger(minLevel, maxLevel));
		}
		else if (type == LogType::File)
		{
			string name = config.Get<string>(loggerTag + ":Name");
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

void LogPool::Init(const std::string& filePath)
{
	_instance = new LogPool(filePath);
}

void LogPool::Uninit()
{
	if (_instance != NULL)
	{
		delete _instance;
	}
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

LogType LogPool::ReadType(const FileConfig& config, const string& key)
{
	string value = config.Get<string>(key);
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

LogLevel LogPool::ReadLevel(const FileConfig& config, const string& key)
{
	string value = config.Get<string>(key);
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
