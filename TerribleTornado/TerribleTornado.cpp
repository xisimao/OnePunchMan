#include "FlowStartup.h"
#include "EventStartup.h"

using namespace std;
using namespace OnePunchMan;

//流量检测缓存
class FlowDetectCache1
{
public:
	FlowDetectCache1()
		:HitPoint()
	{

	}

	//检测点
	Point HitPoint;
};

//流量车道缓存
class FlowLaneCache1
{
public:
	FlowLaneCache1()
		: LaneId(), Region(), MeterPerPixel(0.0)
		, Persons(0), Bikes(0), Motorcycles(0), Cars(0), Tricycles(0), Buss(0), Vans(0), Trucks(0)
		, TotalDistance(0.0), TotalTime(0)
		, TotalInTime(0)
		, LastInRegion(0), Vehicles(0), TotalSpan(0)
		, IoStatus(false), Flag(false)
	{

	}

	//车道编号
	std::string LaneId;
	//当前检测区域
	OnePunchMan::Polygon Region;
	//每个像素代表的米数
	double MeterPerPixel;

	//行人流量
	int Persons;
	//自行车流量
	int Bikes;
	//摩托车流量
	int Motorcycles;
	//轿车流量
	int Cars;
	//三轮车流量
	int Tricycles;
	//公交车流量
	int Buss;
	//面包车流量
	int Vans;
	//卡车流量
	int Trucks;

	//用于计算平均速度
	//车辆行驶总距离(像素值) 
	double TotalDistance;
	//车辆行驶总时间(毫秒)
	long long TotalTime;

	//用于计算时间占有率
	//区域占用总时间(毫秒)
	long long TotalInTime;

	//用于计算车头时距
	//上一次有车进入区域的时间戳 
	long long LastInRegion;
	//机动车总数
	int Vehicles;
	//车辆进入区域时间差的和(毫秒)
	long long TotalSpan;

	//io状态
	bool IoStatus;

	//指针指向实际位置的交替变化标志
	bool Flag;
	//车道内检测项集合
	std::map<std::string, FlowDetectCache1> Items1;
	std::map<std::string, FlowDetectCache1> Items2;

	/**
	* @brief: 获取本次检测项的集合指针
	* @return: 本次检测项的集合指针
	*/
	std::map<std::string, FlowDetectCache1>* CurrentItems()
	{
		return Flag ? &Items1 : &Items2;
	}

	/**
	* @brief: 获取上一次检测项的集合指针
	* @return: 上一次检测项的集合指针
	*/
	std::map<std::string, FlowDetectCache1>* LastItems()
	{
		return Flag ? &Items2 : &Items1;
	}

	/**
	* @brief: 交换当前和上一次的指针
	*/
	void SwitchFlag()
	{
		Flag = !Flag;
	}
};
int main()
{
	vector<FlowLaneCache1> caches1;
	FlowLaneCache1 f1;
	f1.Bikes = 1;
	FlowDetectCache1 c1;
	c1.HitPoint = Point(1, 2);
	f1.Items1.insert(pair<string, FlowDetectCache1>("1", c1));
	caches1.push_back(f1);

	vector<FlowLaneCache1> cache2;
	cache2.assign(caches1.begin(), caches1.end());
	cache2[0].Items1["1"].HitPoint.X = 2;
	cout << caches1.size() << caches1[0].Items1["1"].HitPoint.ToJson() << endl;
	caches1.assign(cache2.begin(), cache2.end());


	cout << caches1.size() << caches1[0].Items1["1"].HitPoint.ToJson() << endl;
	cout << cache2.size() << cache2[0].Items1["1"].HitPoint.ToJson() << endl;


    return 0;
}

int main1(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("stop") == 0)
        {
            SocketMaid maid(2);
            SocketHandler handler;
            maid.AddConnectEndPoint(EndPoint("192.168.201.139", 7772), &handler);
            maid.Start();
            this_thread::sleep_for(chrono::seconds(5));
            stringstream ss;
            ss << "GET /api/system HTTP/1.1\r\n"
                << "\r\n"
                << "\r\n";
            maid.SendTcp(EndPoint("192.168.201.139", 7772), ss.str());
        }
        else if (arg.compare("debug") == 0)
        {
            FFmpegChannel::InitFFmpeg();
            DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
            if (!DecodeChannel::InitHisi(FlowStartup::ChannelCount))
            {
                return -1;
            }
            if (!SeemmoSDK::Init())
            {
                return -1;
            }
            int channelIndex = 1;
            if (argc > 2)
            {
                channelIndex = StringEx::Convert<int>(argv[2]);
            }
            FlowDetector detector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, NULL,true);
            FlowChannelData data;
            FlowChannel channel = data.Get(channelIndex);
            detector.UpdateChannel(channel);
            vector<TrafficDetector*> detectors;
            detectors.push_back(&detector);
            RecognChannel recogn(0, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, detectors);
            DetectChannel detect(1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, &recogn, &detector);
            DecodeChannel decode(channel.ChannelUrl,string(), channel.ChannelIndex, &detect, true);
            recogn.Start();
            detect.Start();
            decode.Start();
            decode.Join();
            detect.Stop();
            recogn.Stop();
            SeemmoSDK::Uninit();
            DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
            DecodeChannel::UninitFFmpeg();
        }
    }
    else
    {
        FlowStartup channel;
        if (channel.Init())
        {
            channel.Start();
            channel.Join();
        }
        else
        {
            LogPool::Information("init flow system failed");
        }
    }

    return 0;
}