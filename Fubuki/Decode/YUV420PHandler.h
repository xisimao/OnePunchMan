#pragma once
#include <stdio.h>

#include "StringEx.h"

namespace Fubuki
{
	class YUV420PHandler
	{
	public:
		YUV420PHandler(int count = 10);

		void HandleFrame(unsigned char* yuv, int width,int height,int packetIndex);

	private:
		int _index;
		int _count;

	};
}


