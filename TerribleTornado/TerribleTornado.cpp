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
	FILE* file = fopen("C:\\Users\\Administrator\\Desktop\\8babb6c1-c1cc-451f-bcce-40003d983883.mp4", "rb");
	unsigned char buffer[1024];
	string base64;
	if (file != NULL)
	{
		unsigned int size = 0;
		while ((size = fread(buffer, 1, 1024, file)) != 0)
		{
			StringEx::ToBase64String(buffer, size, &base64);
		}
		fclose(file);
	}
	LogPool::Information(base64);
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