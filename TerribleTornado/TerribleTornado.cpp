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

//LaneDetector* GetDetector()
//{
//    int screenWidth = 1920;
//    int screenHeight = 1080;
//
//    int laneHeight = 300;
//    Lane lane;
//    lane.LaneIndex = 1;
//    lane.LaneId = "id";
//    lane.LaneName = "lane1";
//    lane.ChannelIndex = 1;
//    lane.DetectLine = "[[410,390],[1210,390]]";
//    lane.StopLine = "[[410,690],[1210,690]]";
//    lane.LaneLine1 = "[[510,290],[510,790]]";
//    lane.LaneLine2 = "[[1110,290],[1110,790]]";
//    lane.Length = 300;
//
//    lane.Direction = 1;
//    lane.FlowDirection = 1;
//    lane.IOIndex = 0;
//    lane.IOIp = "1.1.1.1";
//    lane.IOPort = 80;
//    lane.LaneType = 1;
//
//    return new LaneDetector(lane.LaneId, lane.LaneIndex, lane.GetRegion(), lane.GetMeterPerPixel());
//}
//
//void TestHeadDistance()
//{
//    srand((int)time(0));
//    LaneDetector* detector = GetDetector();
//    vector<LaneDetector*> lanes;
//    lanes.push_back(detector);
//    int t = 0;
//    int count = rand() % 100;
//    long long timeStamp = 1;
//    for (int i = 0; i < count; ++i)
//    {
//        map<string, DetectItem> tempItems1;
//        tempItems1.insert(pair<string, DetectItem>(StringEx::ToString(i), DetectItem(Saitama::Rectangle(809, 539, 2, 2), 4)));
//        detector->Detect(tempItems1, timeStamp);
//        int span = rand() % 500;
//        if (i < count - 1)
//        {
//            t += span;
//            timeStamp += span;
//        }
//
//    }
//    LaneItem item = detector->Collect();
//
//    cout << item.HeadDistance << " " << t / (count - 1) / 1000.0 << endl;
//
//    delete detector;
//}
//
//void TestSpeed()
//{
//    int screenHeight = 1080;
//
//    LaneDetector* detector = GetDetector();
//    vector<LaneDetector*> lanes;
//    lanes.push_back(detector);
//
//    int itemX = 809;
//    int itemY = 0;
//    int itemWidth = 2;
//    int itemHeight = 2;
//    long long timeStamp = 0;
//    timeStamp += 60000;
//    while (itemY < screenHeight)
//    {
//        Saitama::Rectangle rec(itemX, itemY, itemWidth, itemHeight);
//        DetectItem item(rec, 4);
//        map<string, DetectItem> items;
//        items.insert(pair<string, DetectItem>("1", item));
//        detector->Detect(items, timeStamp);
//
//        itemY += 1;
//        timeStamp += 200;
//        if (timeStamp % 60000 == 0)
//        {
//            LaneItem item = detector->Collect();
//            cout << item.Speed << endl;
//        }
//    }
//}
//
//void TestTimeOccupancy()
//{
//    srand((int)time(0));
//    LaneDetector* detector = GetDetector();
//    vector<LaneDetector*> lanes;
//    lanes.push_back(detector);
//    int t = 0;
//    int index = 0;
//    long long timeStamp = 1;
//
//    while (true)
//    {
//        if (timeStamp > 60000)
//        {
//            break;
//        }
//        map<string, DetectItem> tempItems1;
//        tempItems1.insert(pair<string, DetectItem>(StringEx::ToString(index), DetectItem(Saitama::Rectangle(809, 539, 2, 2), 4)));
//        detector->Detect(tempItems1, timeStamp);
//        int span = rand() % 100;
//        t += span;
//        timeStamp += span;
//        detector->Detect(tempItems1, timeStamp);
//        tempItems1.clear();
//        detector->Detect(tempItems1, timeStamp);
//        span = rand() % 100;
//        timeStamp += span;
//        index += 1;
//    }
//    LaneItem item = detector->Collect();
//
//    cout << item.TimeOccupancy << " " << t / 60000.0 * 100 << endl;
//
//    delete detector;
//}
//
//int main1()
//{
//    TestHeadDistance();
//    TestSpeed();
//    TestTimeOccupancy();
//    system("pause");
//
//    return 0;
//}

int main1()
{
    float m1, c1, m2, c2;
    float x1, y1, x2, y2;
    float dx, dy;
    float intersection_X, intersection_Y;

    std::cout << " Program to find the intersecting point of two lines:\n";

    std::cout << "Enter Line1 - X1: ";
    std::cin >> x1;

    std::cout << "Enter Line1 - Y1: ";
    std::cin >> y1;

    std::cout << "Enter Line1 - X2: ";
    std::cin >> x2;

    std::cout << "Enter Line1 - Y2: ";
    std::cin >> y2;

    dx = x2 - x1;
    dy = y2 - y1;

    m1 = dy / dx;
    // y = mx + c
    // intercept c = y - mx
    c1 = y1 - m1 * x1; // which is same as y2 - slope * x2


    std::cout << "Enter Line2 - X1: ";
    std::cin >> x1;

    std::cout << "Enter Line2 - Y1: ";
    std::cin >> y1;

    std::cout << "Enter Line2 - X2: ";
    std::cin >> x2;

    std::cout << "Enter Line2 - Y2: ";
    std::cin >> y2;

    dx = x2 - x1;
    dy = y2 - y1;

    m2 = dy / dx;
    c2 = y2 - m2 * x2; // which is same as y2 - slope * x2

    std::cout << "Equation of line1: ";
    std::cout << m1 << "X " << ((c1 < 0) ? ' ' : '+') << c1 << "\n";

    std::cout << "Equation of line2: ";
    std::cout << m2 << "X " << ((c2 < 0) ? ' ' : '+') << c2 << "\n";

    if ((m1 - m2) == 0)
        std::cout << "No Intersection between the lines\n";
    else
    {
        intersection_X = (c2 - c1) / (m1 - m2);
        intersection_Y = m1 * intersection_X + c1;
        std::cout << "Intersecting Point: = ";
        std::cout << intersection_X;
        std::cout << ",";
        std::cout << intersection_Y;
        std::cout << "\n";
    }
    system("pause");
    return 0;
}

int main2()
{
    Point p1(358, 625);
    Point p2(791, 625);
    Point p3(358, 608);
    Point p4(358, 900);
    float x1 = p1.X, x2 = p2.X, x3 = p3.X, x4 = p4.X;
    float y1 = p1.Y, y2 = p2.Y, y3 = p3.Y, y4 = p4.Y;

    float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // If d is zero, there is no intersection
    if (d == 0) return 0;

    // Get the x and y
    float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
    float x = round((pre * (x3 - x4) - (x1 - x2) * post) / d);
    float y = round((pre * (y3 - y4) - (y1 - y2) * post) / d);

    // Check if the x and y coordinates are within both lines
    if (x < min(x1, x2) || x > max(x1, x2) ||
        x < min(x3, x4) || x > max(x3, x4)) return 0;
    if (y < min(y1, y2) || y > max(y1, y2) ||
        y < min(y3, y4) || y > max(y3, y4)) return 0;

    // Return the point of intersection

    Point p(x, y);

    cout << "123";

    return 0;
}

class TestHandler:public SocketHandler
{

protected:

    SocketHandler* CloneCore()
    {
        return new TestHandler();
    }

    ProtocolPacket HandleCore(int socket, unsigned int ip, unsigned short port, std::string::const_iterator begin, std::string::const_iterator end)
    {
        string s(begin, end);
        cout << s;
        return ProtocolPacket(AnalysisResult::Request, 0, end - begin,0, 0);
    }

};

int main()
{
   ///* stringstream ss;
   // ss << "OPTIONS /api/channels?timestamp=1585101111002 HTTP/1.1\r\n"
   //     << "Host: 192.168.201.111:7772\r\n"
   //     << "Connection: keep-alive\r\n"
   //     << "Access-Control-Request-Method: POST\r\n"
   //     << "Origin: http://localhost:8092\r\n"
   //     << "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36\r\n"
   //     << "Access-Control-Request-Headers: authorization,content-type\r\n"
   //     << "Accept: */*\r\n"
   //     << "Referer: http://localhost:8092/\r\n"
   //     << "Accept-Encoding: gzip, deflate\r\n"
   //     << "Accept-Language: zh-CN,zh;q=0.9\r\n"
   //     << "\r\n"
   //     << "\r\n";



   // SocketMaid maid(2);
   // TestHandler handler1;

   // maid.AddConnectEndPoint(EndPoint("127.0.0.1",7777), &handler1);
   // maid.Start();

   // system("pause");

   // maid.SendTcp(EndPoint("127.0.0.1", 7777), ss.str());
   // system("pause");

   // maid.Stop();*/

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

