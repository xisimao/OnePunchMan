#pragma once
#include <string>
#include <bitset>

#include "Thread.h"

extern "C"
{
#include "libavformat/avformat.h"
}

namespace Fubuki
{
	class FrameChannel :public Saitama::ThreadObject
	{
	public:
		FrameChannel();

		virtual ~FrameChannel() {}

		static void InitFFmpeg();

		static void UninitFFmpeg();

		int InitFile(const std::string& url, bool sync, bool loop);

		int InitRtsp(const std::string& url);

	protected:
		void StartCore();

		virtual int InitCore() { return 0; };

		virtual void UninitCore() {};

		virtual bool Decode(const AVPacket* packet, int packetIndex) { return true; }

		//video
		std::string _url;
		AVFormatContext* _formatContext;
		int _videoIndex;



	private:
		int Init();

		void Uninit();

		//init
		AVDictionary* _options;
		bool _sync;
		bool _loop;
	};

}

