#pragma once
#include <stdio.h>

namespace Fubuki
{
	class H264Handler
	{
	public:
		H264Handler(int count=100);

		~H264Handler();

		void HandleFrame(unsigned char* frame, int size);

	private:
		int _index;
		int _count;

		FILE* _h264File;

	};
}
