#pragma once

//��־�¼�
enum class LogEvent :int
{
	None = 0,
	System = 1,
	Socket=2,
	Mqtt = 3,
	Sqlite=4,
	Encode =5,
	Decode =6,
	Detect=7,
	Recogn=8,
	Flow=9,
	Event=10,
	Adapter=11,
	Http=12,
	Monitor=13
};