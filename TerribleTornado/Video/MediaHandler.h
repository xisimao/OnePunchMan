#pragma once
#include "StringEx.h"
#include "MqttChannel.h"

namespace TerribleTornado
{
	class MediaHandler
	{
	public:
		MediaHandler(MqttChannel* mqtt,int channelIndex);

		void HandleFrame(unsigned char* frame, int size);

	private:

		MqttChannel* _mqtt;

		std::string _mediaTopic;
	};

}

