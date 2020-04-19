#pragma once
#include <stdio.h>

#include "StringEx.h"

namespace Fubuki
{
	class IVE_8UC3Handler
	{
	public:
		IVE_8UC3Handler(int count = 3);

		void HandleFrame(unsigned char* ive, int width, int height, int packetIndex);

	private:

		int _index;
		int _count;
	};
}


