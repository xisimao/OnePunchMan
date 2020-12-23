#include "TrafficStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"
#include "SqliteLogger.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    string ip = "192.168.2.100";
    int channelIndex = 1;

    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    Socket::Init();
    SocketMaid maid(2, true);
    SocketHandler handler;
    maid.AddConnectEndPoint(EndPoint(ip, 7772), &handler);
    maid.Start();
    TrafficChannel channel;
    while (true)
    {
        stringstream ss;
        ss << "GET /api/channels/"<<channelIndex<<" HTTP/1.1\r\n"
            << "Host: 127.0.0.1:7772\r\n"
            << "\r\n";

        string httpProtocol;
        SocketResult result = maid.SendTcp(EndPoint(ip, 7772), ss.str(), 0, 0, &httpProtocol);
        if (result == SocketResult::Success)
        {
            vector<string> lines = StringEx::Split(httpProtocol, "\r\n");
            JsonDeserialization jd(lines[lines.size() - 1]);
            channel.ChannelIndex = jd.Get<int>("channelIndex");
            channel.ChannelName = jd.Get<string>("channelName");
            channel.ChannelUrl = jd.Get<string>("channelUrl");
            channel.ChannelType = jd.Get<int>("channelType");
            channel.LaneWidth = jd.Get<double>("laneWidth");
            channel.DeviceId = jd.Get<string>("deviceId");
            channel.Loop = jd.Get<bool>("loop");
            channel.OutputImage = jd.Get<bool>("outputImage");
            channel.OutputReport = jd.Get<bool>("outputReport");
            channel.OutputRecogn = jd.Get<bool>("outputRecogn");
            channel.GlobalDetect = jd.Get<bool>("globalDetect");
            int flowLaneIndex = 0;
            while (true)
            {
                FlowLane lane;
                lane.LaneId = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneId"));
                if (lane.LaneId.empty())
                {
                    break;
                }
                lane.ChannelIndex = channel.ChannelIndex;
                lane.LaneName = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneName"));
                lane.LaneIndex = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneIndex"));
                lane.Direction = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":direction"));
                lane.FlowDirection = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":flowDirection"));
                lane.StopLine = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":stopLine"));
                lane.FlowDetectLine = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":flowDetectLine"));
                lane.QueueDetectLine = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":queueDetectLine"));
                lane.LaneLine1 = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneLine1"));
                lane.LaneLine2 = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":laneLine2"));
                lane.IOIp = jd.Get<string>(StringEx::Combine("flowLanes:", flowLaneIndex, ":ioIp"));
                lane.IOPort = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":ioPort"));
                lane.IOIndex = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":ioIndex"));
                lane.ReportProperties = jd.Get<int>(StringEx::Combine("flowLanes:", flowLaneIndex, ":reportProperties"));

                //构建检测区域
                BrokenLine laneLine1 = BrokenLine::FromJson(lane.LaneLine1);
                BrokenLine laneLine2 = BrokenLine::FromJson(lane.LaneLine2);
                Line stopLine = Line::FromJson(lane.StopLine);
                Line flowDetectLine = Line::FromJson(lane.FlowDetectLine);
                Line queueDetectLine = Line::FromJson(lane.QueueDetectLine);
                lane.FlowRegion = Polygon::Build(laneLine1, laneLine2, stopLine, flowDetectLine).ToJson();
                lane.QueueRegion = Polygon::Build(laneLine1, laneLine2, stopLine, queueDetectLine).ToJson();

                channel.FlowLanes.push_back(lane);
                flowLaneIndex += 1;
            }
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    maid.Stop();

    //TrafficData data("C:\\Users\\Administrator\\Desktop\\traffic.db");
    //TrafficChannel channel=data.GetChannel(channelIndex);
    channel.OutputImage = true;
    vector<FlowDetector*> detectors;
    for (int i = 0; i < 8; ++i)
    {
        FlowDetector* detector = new FlowDetector(1920, 1080, NULL, NULL);
        if (i == channelIndex - 1)
        {
            detector->UpdateChannel(1,channel);
            detector->ResetTimeRange();
        } 
        detectors.push_back(detector);
    }

    //文件流 
    ifstream file;
    file.open("D:\\code\\OnePunchMan\\data\\Traffic_20191223.log", ofstream::in);
    string s;
    while (getline(file, s))
    {
        if (s.size() > 33)
        {
            string time = s.substr(1, 23);
            string level = s.substr(26, 1);
            int eventBegin = s.find('[', 26);
            if (eventBegin == string::npos)
            {
                continue;
            }
            int eventEnd = s.find(']', eventBegin);
            if (eventEnd == string::npos)
            {
                continue;
            }
            string eventId = s.substr(eventBegin + 1, eventEnd - eventBegin - 1);
            if (eventId.compare("13")==0)
            {
                string content = s.substr(eventEnd + 2, s.size() - eventEnd - 2); 
                JsonDeserialization jd(content);
                int channelIndex = jd.Get<int>("channelIndex");
                int frameIndex = jd.Get<int>("frameIndex");
                int frameSpan = jd.Get<int>("frameSpan");
                int itemIndex = 0;
                map<string, DetectItem> items;
                while (true)
                {
                    DetectItem item;
                    item.Id = jd.Get<string>(StringEx::Combine("items:", itemIndex, ":id"));
                    if (item.Id.empty())
                    {
                        break;
                    }
                    item.Type = static_cast<DetectType>(jd.Get<int>(StringEx::Combine("items:", itemIndex, ":type")));
                    item.Region = OnePunchMan::Rectangle::FromJson(jd.Get<string>(StringEx::Combine("items:", itemIndex, ":region")));
                    items.insert(pair<string, DetectItem>(item.Id, item));
                    itemIndex += 1;
                }
                detectors[channelIndex - 1]->HandleDetect(&items, frameIndex*frameSpan, 1, NULL, frameIndex, frameSpan);
            }
        }
    }
    file.close();


    //TrafficStartup startup;
    //startup.Start();
    //startup.Join();


    return 0;
}
