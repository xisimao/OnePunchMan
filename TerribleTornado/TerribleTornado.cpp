#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"
#include "SqliteLogger.h"
#include <list>

using namespace std;
using namespace OnePunchMan;


//车辆的距离
class CarDistance
{
public:
    CarDistance()
        :Type(DetectType::None), Distance(0), HitPoint()
    {

    }
    DetectType Type;
    double Distance;
    Point HitPoint;
};

void OrderedList(list<CarDistance>* l, const CarDistance& i)
{
    if (l->empty())
    {
        l->push_back(i);
    }
    else
    {
        for (list<CarDistance>::iterator it = l->begin(); it != l->end(); ++it)
        {
            if (it->Distance > i.Distance)
            {
                list<CarDistance>::iterator temp =  l->insert(it, i);
                return;
            }
        }
        l->push_back(i);
    }
}


double Fun(const list<CarDistance>& distances)
{
    long long d = 0.0;
    //计算排队长度
    if (distances.size() > 1)
    {
        list<CarDistance>::const_iterator preCar = distances.begin();
        list<CarDistance>::const_iterator nextCar = ++preCar;
        while (preCar != distances.end() && nextCar != distances.end())
        {
            if (nextCar->Distance - preCar->Distance > 100)
            {
                break;
            }
            else
            {
                if (d == 0.0)
                {
                    d += static_cast<int>(preCar->Type);
                }

                d += static_cast<int>(nextCar->Type);
            }
            preCar = nextCar;
            ++nextCar;
        }
    }
    return d;
}

int main(int argc, char* argv[])
{
    list<CarDistance> l;
    CarDistance d1;
    d1.Distance = 3.3;
    d1.Type = DetectType::Pedestrain;

    CarDistance d2;
    d2.Distance = 5.3;
    d2.Type = DetectType::Pedestrain;
    CarDistance d3;
    d3.Distance = 4.3;
    d3.Type = DetectType::Pedestrain;
    CarDistance d4;
    d4.Distance = 8;
    d4.Type = DetectType::Pedestrain;
    CarDistance d5;
    d5.Distance = 2;
    d5.Type = DetectType::Pedestrain;

    OrderedList(&l, d1);
    OrderedList(&l, d2);
    OrderedList(&l, d3);
    OrderedList(&l, d4);
    OrderedList(&l, d5);

   double d= Fun(l);
    system("pause");

    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("flow") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            TrafficDirectory::Init(jd, jd.Get<string>("Directory:FlowWeb"));
            TrafficData::Init(jd.Get<string>("Flow:Db"));
            SqliteLogger logger(TrafficData::DbName);
            LogPool::AddLogger(&logger);

            FlowStartup startup;
            startup.Start();
            startup.Join();
        }
        else if (arg.compare("event") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            TrafficDirectory::Init(jd, jd.Get<string>("Directory:EventWeb"));
            TrafficData::Init(jd.Get<string>("Event:Db"));
            SqliteLogger logger(TrafficData::DbName);
            LogPool::AddLogger(&logger);
            EventStartup startup;
            startup.Start();
            startup.Join();
        }
        else if (arg.compare("io") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            TrafficDirectory::Init(jd, jd.Get<string>("Directory:FlowWeb"));
            TrafficData::Init(jd.Get<string>("Flow:Db"));

            IoAdapter adapter;
            adapter.Start();
            adapter.Join();
        }
    }
    //JsonDeserialization jd("appsettings.json");
    //LogPool::Init(jd);
    //TrafficDirectory::Init(jd);
    //TrafficData::Init(jd.Get<string>("Flow:Db"));
    //SqliteLogger logger(TrafficData::DbName);
    //LogPool::AddLogger(&logger);
    //DecodeChannel::InitFFmpeg();
    //DecodeChannel channel(1, -1, NULL);
    //channel.UpdateChannel("rtsp://admin:Hikjxjj123@66.66.12.86/media/video1/multicast", "", ChannelType::RTSP, true);
    //channel.Start();
    //channel.Join();

    return 0;
}
