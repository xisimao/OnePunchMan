#pragma once

//��־�¼�
enum class LogEvent :int
{
	None = 0,
	Thread = 1,
	Socket=2,
	Mqtt = 3,
	Sqlite=4,
	Detect=5
};