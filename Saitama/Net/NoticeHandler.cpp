#include "NoticeHandler.h"

using namespace std;
using namespace Saitama;

NoticeHandler::NoticeHandler(unsigned int protocolId)
	:NoticeHandler(0,protocolId, NULL)
{
}


NoticeHandler::NoticeHandler(unsigned int protocolId, string* buffer)
	: NoticeHandler(0, protocolId,buffer)
{
}

NoticeHandler::NoticeHandler(long long timeStamp, unsigned int protocolId)
	: NoticeHandler(timeStamp, protocolId, NULL)
{

}

NoticeHandler::NoticeHandler(long long timeStamp, unsigned int protocolId, string* buffer)
	:AsyncHandler(timeStamp,protocolId)
{
	_isCompleted = false;
	_buffer = buffer;
}

bool NoticeHandler::IsCompleted()
{
	return _isCompleted;
}

bool NoticeHandler::Wait(int milliseconds)
{
	unique_lock<mutex> lck(_mutex);
	cv_status status = _condition.wait_for(lck, chrono::milliseconds(milliseconds));
	_isCompleted = true;
	return status == cv_status::no_timeout;
}

void NoticeHandler::Noticty()
{
	unique_lock<mutex> lck(_mutex);
	_condition.notify_all();
}

void NoticeHandler::Handle(string::const_iterator begin, string::const_iterator end)
{
	if (_buffer != NULL)
	{
		_buffer->assign(begin,end);
	}
	Noticty();
	_isCompleted = true;
}