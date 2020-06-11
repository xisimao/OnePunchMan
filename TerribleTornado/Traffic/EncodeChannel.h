#pragma once
#include "Thread.h"

namespace OnePunchMan
{
	class EncodeChannel:ThreadObject
	{
	public:
		EncodeChannel();

	protected:
		void StartCore();
	};
}


