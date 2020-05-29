#include "Thread.h"

using namespace std;
using namespace OnePunchMan;

const int ThreadObject::LockTime = 3;
const int ThreadObject::SleepTime = 100;

ThreadObject::ThreadObject(const string& name)
	: _cancelled(false),_name(name), _status(ThreadStatus::Unstarted)
{

}

ThreadObject::~ThreadObject()
{
	Stop();
	if (_thread.joinable())
	{
		_thread.detach();
	}
}

string ThreadObject::Name() const
{
	return _name;
}

void ThreadObject::Start()
{
	if (_status == ThreadStatus::Unstarted)
	{		
		thread t(&ThreadObject::StartThread, this);
		_thread.swap(t);
		while (_status == ThreadStatus::Unstarted)
		{
			this_thread::sleep_for(chrono::milliseconds(SleepTime));
		}
	}
}

void ThreadObject::StartThread()
{
	_status = ThreadStatus::Running;
	LogPool::Information(LogEvent::System,"start thread", _name);
	StartCore();
	LogPool::Information(LogEvent::System,"stop thread",_name);
	_status = ThreadStatus::Stopped;
}

void ThreadObject::Join()
{
	if (_thread.joinable())
	{
		_thread.join();
	}
}

void ThreadObject::Stop()
{
	_cancelled = true;
	StopCore();
	while (_status == ThreadStatus::Running)
	{
		this_thread::sleep_for(chrono::milliseconds(SleepTime));
	}
}