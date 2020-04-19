#include "MediaHandler.h"

using namespace Saitama;
using namespace TerribleTornado;

MediaHandler::MediaHandler(MqttChannel* mqtt,int channelIndex)
	:_mqtt(mqtt)
{
	_mediaTopic = StringEx::Combine("Stream_", channelIndex);
}

void MediaHandler::HandleFrame(unsigned char* frame, int size)
{
	_mqtt->Send(_mediaTopic, frame, size);
}
