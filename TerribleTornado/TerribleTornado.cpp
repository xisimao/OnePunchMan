#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "YUV420PHandler.h"
#include "EncodeHandler.h"

using namespace std;
using namespace OnePunchMan;

int main()
{
	LogPool::Init("appsettings.json");
	FFmpegChannel::InitFFmpeg();
	int yuvSize = 1920 * 1080 * 1.5;
	unsigned char* yuvBuffer = new unsigned char[yuvSize];
	string json;
	EncodeHandler handler1(&json, "temp1.mp4", 1920, 1080,3);
	EncodeHandler handler2(&json, "temp2.mp4", 1920, 1080,3);
	EncodeHandler handler3(&json, "temp3.mp4", 1920, 1080,3);

	for (int j = 0; j < 3*250; ++j)
	{
		for (int i = 1; i <= 1; ++i)
		{
			FILE* file = fopen(StringEx::Combine("D:\\code\\OnePunchMan\\images\\yuv420p_", i, ".yuv").c_str(), "rb");
			if (file != NULL)
			{
				fread(yuvBuffer, 1, yuvSize, file);
				handler1.AddYuv(yuvBuffer);
				handler2.AddYuv(yuvBuffer);
				handler3.AddYuv(yuvBuffer);
				fclose(file);
			}
		}
	}

	delete[] yuvBuffer;
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