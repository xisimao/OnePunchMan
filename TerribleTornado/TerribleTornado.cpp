#include <stdio.h>
#include <stdlib.h>
#include "mosquitto.h"
#include <string.h>
#include "LogPool.h"
#include <string>
#include <iostream>
#include <fstream>
#include "LogReader.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "DataChannel.h"
#include "ByteFormatter.h"
#include "Sqlite.h"
#include "FlowChannelData.h"
#include "SocketMaid.h"
#include "HttpHandler.h"

using namespace std;
using namespace Saitama;

LaneDetector* GetDetector()
{
    int screenWidth = 1920;
    int screenHeight = 1080;

    int laneHeight = 300;
    Lane lane;
    lane.LaneIndex = 1;
    lane.LaneId = "id";
    lane.LaneName = "lane1";
    lane.ChannelIndex = 1;
    lane.DetectLine = "[[410,390],[1210,390]]";
    lane.StopLine = "[[410,690],[1210,690]]";
    lane.LaneLine1 = "[[510,290],[510,790]]";
    lane.LaneLine2 = "[[1110,290],[1110,790]]";
    lane.Length = 300;

    lane.Direction = 1;
    lane.FlowDirection = 1;
    lane.IOIndex = 0;
    lane.IOIp = "1.1.1.1";
    lane.IOPort = 80;
    lane.LaneType = 1;

    return new LaneDetector("1_1",lane);
}

void TestHeadDistance()
{
    srand((int)time(0));
    LaneDetector* detector = GetDetector();
    vector<LaneDetector*> lanes;
    lanes.push_back(detector);
    int t = 0;
    int count = rand() % 100;
    long long timeStamp = 1;
    for (int i = 0; i < count; ++i)
    {
        map<string, DetectItem> tempItems1;
        tempItems1.insert(pair<string, DetectItem>(StringEx::ToString(i), DetectItem(Saitama::Rectangle(809, 539, 2, 2), 4)));
        detector->Detect(tempItems1, timeStamp);
        int span = rand() % 500;
        if (i < count - 1)
        {
            t += span;
            timeStamp += span;
        }

    }
    LaneItem item = detector->Collect(timeStamp);

    cout << item.HeadDistance << " " << t / (count - 1) / 1000.0 << endl;

    delete detector;
}

void TestSpeed()
{
    int screenHeight = 1080;

    LaneDetector* detector = GetDetector();
    vector<LaneDetector*> lanes;
    lanes.push_back(detector);

    int itemX = 809;
    int itemY = 0;
    int itemWidth = 2;
    int itemHeight = 2;
    long long timeStamp = 0;
    timeStamp += 60000;
    while (itemY < screenHeight)
    {
        Saitama::Rectangle rec(itemX, itemY, itemWidth, itemHeight);
        DetectItem item(rec, 4);
        map<string, DetectItem> items;
        items.insert(pair<string, DetectItem>("1", item));
        detector->Detect(items, timeStamp);

        itemY += 1;
        timeStamp += 200;
        if (timeStamp % 60000 == 0)
        {
            LaneItem item = detector->Collect(timeStamp);
            cout << item.Speed << endl;
        }
    }
}

void TestTimeOccupancy()
{
    srand((int)time(0));
    LaneDetector* detector = GetDetector();
    vector<LaneDetector*> lanes;
    lanes.push_back(detector);
    int t = 0;
    int index = 0;
    long long timeStamp = 1;

    while (true)
    {
        if (timeStamp > 60000)
        {
            break;
        }
        map<string, DetectItem> tempItems1;
        tempItems1.insert(pair<string, DetectItem>(StringEx::ToString(index), DetectItem(Saitama::Rectangle(809, 539, 2, 2), 4)));
        detector->Detect(tempItems1, timeStamp);
        int span = rand() % 100;
        t += span;
        timeStamp += span;
        detector->Detect(tempItems1, timeStamp);
        tempItems1.clear();
        detector->Detect(tempItems1, timeStamp);
        span = rand() % 100;
        timeStamp += span;
        index += 1;
    }
    LaneItem item = detector->Collect(timeStamp);

    cout << item.TimeOccupancy << " " << t / 60000.0 * 100 << endl;

    delete detector;
}

int main1()
{
    TestHeadDistance();
    TestSpeed();
    TestTimeOccupancy();
    system("pause");

    return 0;
}

int main()
{
    SocketMaid maid(2);
    DataChannel channel("192.168.201.139", 1884);
    HttpHandler handler;
    handler.HttpReceived.Subscribe(&channel);
    maid.AddListenEndPoint(EndPoint(7772), &handler);
    channel.Start();
    maid.Start();

    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
    }

    maid.Stop();
    channel.Stop();

    return 0;
}

