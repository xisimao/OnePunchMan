#pragma once

//��־�¼�
enum class LogEvent :int
{
	None = 0,
	Thread = 1,
	Mqtt = 2,
	Sqlite=3,
	Detect=4
};