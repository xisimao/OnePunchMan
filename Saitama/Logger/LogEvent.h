#pragma once

//日志事件
enum class LogEvent :int
{
	None = 0,
	Thread = 1,
	Socket=2,
	Mqtt = 3,
	Sqlite=4,
	Decode =5,
	Detect=6,
	Recogn=7,
	Flow=8,
	Event=9,
	Adapter=10
};