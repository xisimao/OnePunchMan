#pragma once
#include <queue>
#include <vector>

#include "Lane.h"
#include "Thread.h"

namespace Saitama
{
	class VideoChannel:public ThreadObject
	{
	public:

		void Push(const std::string& message);

	;

	protected:

		void StartCore();

		void Collect(const std::string& message);

	private:

		std::mutex _mutex;

		std::queue<std::string> _messages;

		std::vector<Lane> _lanes;
	};
}


