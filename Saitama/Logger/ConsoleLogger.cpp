#include "ConsoleLogger.h"

using namespace std;
using namespace OnePunchMan;

ConsoleLogger::ConsoleLogger(LogLevel minLevel, LogLevel maxLevel)
	:Logger(minLevel,maxLevel)
{
	
}

void ConsoleLogger::LogCore(LogLevel logLevel, LogEvent logEvent, const DateTime& time, const string& content)
{
	cout<< content;
}