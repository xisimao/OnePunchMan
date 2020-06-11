#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeHandler.h"

using namespace std;
using namespace OnePunchMan;

int main()
{
	LogPool::Init("appsettings.json");
	FFmpegChannel::InitFFmpeg();
	int yuvSize = 1920 * 1080 * 1.5;
	unsigned char* yuv420spBuffer = new unsigned char[yuvSize];
	unsigned char* yuv420pBuffer = new unsigned char[yuvSize];
	string json;
	EncodeHandler handler1(&json, "temp1.mp4", 1920, 1080, 10);

	for (int i = 0; i < 10 * 250; ++i)
	{
		FILE* file = fopen(StringEx::Combine("../images/yuv420sp_", 1, ".yuv").c_str(), "rb");
		if (file != NULL)
		{
			fread(yuv420spBuffer, 1, yuvSize, file);
			//ImageConvert::Yuv420spToYuv420p(yuv420spBuffer, 1920, 1080, yuv420pBuffer);
			handler1.AddYuv(yuv420spBuffer);
			if (handler1.Finished())
			{
				cout << "1" << endl;
			}
			fclose(file);
		}
	}

	delete[] yuv420spBuffer;
	delete[] yuv420pBuffer;
	return 0;
}


int main1(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    FFmpegChannel::InitFFmpeg();
    FFmpegChannel channel(false);
    channel.UpdateChannel("600w.mp4", string());
    channel.Start();
    system("pause");
    channel.Stop();
    return 0;
}