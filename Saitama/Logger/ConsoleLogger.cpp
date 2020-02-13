#include "ConsoleLogger.h"

using namespace std;
using namespace Saitama;

ConsoleLogger::ConsoleLogger(LogLevel minLevel, LogLevel maxLevel)
	:Logger(minLevel,maxLevel)
{
	
}

void ConsoleLogger::LogCore(const std::string& log)
{
	cout<<log;
}