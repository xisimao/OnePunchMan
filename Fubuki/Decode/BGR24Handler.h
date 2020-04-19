#pragma once
#include <stdio.h>

#include "StringEx.h"

namespace Fubuki
{
	class BGR24Handler
	{
	public:
		BGR24Handler(int count=10);

		void HandleFrame(unsigned char* bgr24, int width, int height,int packetIndex);

	private:

		int _index;
		int _count;
	};
}


