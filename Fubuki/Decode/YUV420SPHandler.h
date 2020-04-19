#pragma once
#include "StringEx.h"

namespace Fubuki
{
	class YUV420SPHandler
	{
	public:
		YUV420SPHandler(int count = 3);

		void HandleFrame(unsigned char* yuv, int width, int height,int packetIndex);

	private:
		int _index;
		int _count;
	};

}


