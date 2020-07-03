#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "HisiEncodeChannel.h"
#include "OutputHandler.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    DecodeChannel::InitFFmpeg();
    OutputHandler handler;
    handler.Init(1, "temp.mp4", "600w.mp4",1);
    int size = 1024;
    while(true)
    {
        AVPacket* packet = av_packet_alloc();
        unsigned char* temp = (unsigned char*)av_malloc(size);
        memset(temp, 1, size);
        av_packet_from_data(packet, temp, size);
        av_write_frame(handler._outputFormat, packet);
        av_packet_unref(packet);
        //av_interleaved_write_frame(handler._outputFormat, packet);
        av_packet_free(&packet);
        //this_thread::sleep_for(chrono::milliseconds(10));
    }
    handler.Uninit();
    system("pause");
    //vector<string> columns = StringEx::Split(" 1629  1626 root     S    2116m141.8   3 73.3 ./Genos.out flow", " ", true);
    //if (columns.size() >= 5)
    //{
    //    vector<string> datas=StringEx::Split(columns[4], "m", true);
    //    if (!datas.empty())
    //    {
    //        cout << datas[0] << endl;
    //    }
    //}
    //LogPool::Init("appsettings.json");
    //FlowStartup startup;
    //startup.Start();
    //startup.Join();
    //DecodeChannel::InitFFmpeg();

    //HisiDecodeChannel channel(1);
    //channel.UpdateChannel("600w.mp4", string(), false);
    //HisiEncodeChannel encode;
    //encode.Start();
    //channel.Start();
    //this_thread::sleep_for(chrono::seconds(3));
    //OutputHandler handler;
    //handler.Init(1,"temp.mp4", "600w.mp4",250);
    //unsigned char* data = new unsigned char [1024 * 1024];
    //for (int i =1; i <=100; ++i)
    //{
    //    FILE* file = fopen(StringEx::Combine("../images/h264_", i, ".h264").c_str(), "rb");
    //    size_t size=fread(data, 1, 1024 * 1024, file);
    //    handler.PushPacket(data, size);
    //    fclose(file);
    //}
    //handler.Uninit();
    //while (true)
    //{
    //    this_thread::sleep_for(chrono::seconds(1));
    //}
    system("pause");
    //channel.Stop();
    return 0;
}