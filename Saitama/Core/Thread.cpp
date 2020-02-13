#include "Thread.h"

using namespace std;
using namespace Saitama;

const int ThreadObject::PollTime = 100;

ThreadObject::ThreadObject(const string& name)
	: _name(name), _status(ThreadStatus::Unstarted), _cancelled(false)
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

bool ThreadObject::Cancelled()
{
	_hitPoint = DateTime::Now();
	return _cancelled;
}

DateTime ThreadObject::HitPoint() const
{
	return _hitPoint;
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
			this_thread::sleep_for(chrono::milliseconds(PollTime));
		}
	}
}

void ThreadObject::StartThread()
{
	_status = ThreadStatus::Running;
	LogPool::Information(LogEvent::Thread,"start", _name);
	StartCore();
	LogPool::Information(LogEvent::Thread,"stop",_name);
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
		this_thread::sleep_for(chrono::milliseconds(PollTime));
	}
}