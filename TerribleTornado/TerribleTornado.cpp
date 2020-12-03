#include "TrafficStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"
#include "SqliteLogger.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    BrokeLine brokeLine1;
    brokeLine1.AddPoint(Point(860, 400));
    brokeLine1.AddPoint(Point(670, 1069));
    BrokeLine brokeLine2;
    brokeLine2.AddPoint(Point(1184, 437));
    brokeLine2.AddPoint(Point(1460, 1067));
    Line line1(Point(754, 626),Point(1332, 626));
    Line line2(Point(675, 858),Point(1450, 858));
    Line line3(Point(580, 1042),Point(1555, 1042));
    OnePunchMan::Polygon p=Polygon::Build(brokeLine1, brokeLine2, line1, line2);
    OnePunchMan::Polygon p2=Polygon::Build(brokeLine1, brokeLine2, line1, line3);


    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    //SqliteLogger logger(TrafficData::DbName);
    //LogPool::AddLogger(&logger);
    TrafficStartup startup;
    startup.Start();
    startup.Join();

    return 0;
}
