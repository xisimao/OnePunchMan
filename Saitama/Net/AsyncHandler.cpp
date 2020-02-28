#include "AsyncHandler.h"

using namespace std;
using namespace Saitama;

AsyncHandler::AsyncHandler(long long timeStamp, unsigned int protocolId)
	:_timeStamp(timeStamp),_protocolId(protocolId)
{
}

long long AsyncHandler::TimeStamp()
{
	return _timeStamp;
}

unsigned int AsyncHandler::ProtocolId()
{
	return _protocolId;
}