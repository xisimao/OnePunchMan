#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{

    LogPool::Init("appsettings.json");
    TrafficData::Init("flow.db");
    FFmpegInput::InitFFmpeg();

    H264Cache cache1(1);
    cache1.AddOutputUrl("1.mp4", 8);
    cache1.AddOutputUrl("2.mp4", 8);

    DecodeChannel channel(1, 1, NULL);
    channel.UpdateChannel("600w.mp4", string("rtmp://192.168.1.65:1935/live/9"), ChannelType::File, false);
    channel.Start();

    //RtmpOutput output;
    //FFmpegInput input;
    ////input.Init("600w.mp4");
    //output.Init("rtmp://192.168.1.65:1935/live/9", input);
    //AVPacket* packet = av_packet_alloc();
    //unsigned char* buffer = new unsigned char[10 * 1024 * 1024];
    ////FILE* fw = fopen(StringEx::Combine("C:\\Users\\Administrator\\Desktop\\temp\\h264_1.h264").c_str(), "wb");
    //for (int i = 1; i < 4500; ++i)
    //{
    //    FILE* file = fopen(StringEx::Combine("C:\\Users\\Administrator\\Desktop\\temp\\h264_1_", i, ".h264").c_str(), "rb");
    //    if (file != NULL)
    //    {
    //        unsigned int size = fread(buffer, 1, 10 * 1024 * 1024, file);
    //        //unsigned int wsize=fwrite(buffer, 1, size, fw);
    //        if (i % 12 == 1)
    //        {
    //            packet->flags = 1;
    //        }
    //        else
    //        {
    //            packet->flags = 0;
    //        }
    //        av_packet_from_data(packet, buffer, size);
    //        output.PushRtmpPacket(packet, i, 0);
    //        fclose(file);
    //        this_thread::sleep_for(chrono::milliseconds(40));
    //    }
    //    else
    //    {
    //        cout << endl;
    //    }
    //}
    //av_packet_free(&packet);
    //delete[] buffer;
    //fclose(fw);
    system("pause");

    return 0;
}
