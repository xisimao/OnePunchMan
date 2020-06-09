#pragma once

//日志事件
enum class LogEvent :int
{
	None = 0,
	System = 1,
	Socket=2,
	Mqtt = 3,
	Sqlite=4,
	Decode =5,
	Encode=6,
	Detect=7,
	Recogn=8,
	Flow=9,
	Event=10,
	Adapter=11,
	Http=12
};