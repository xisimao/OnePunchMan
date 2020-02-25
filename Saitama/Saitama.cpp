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
#include "Speed.h"
#include "MqttChannel.h"

using namespace std;
using namespace Saitama;

class BodyItem
{
public:
    vector<int> Rect;
};

class DetectItem
{
public:
    BodyItem Body;
};

class VehiclesItem
{
public:
    int Type;
    std::string GUID;
    DetectItem Detect;
};

class ImageResultsItem
{
public:
    std::vector< VehiclesItem> Vehicles;
};

class Item
{
public:
    ImageResultsItem ImageResults;
    int Code;
    std::string Message;
};

class Test :public ThreadObject,public IObserver<MqttReceivedEventArgs>
{
public:

    Test()
        :ThreadObject("test")
    {

    }

    void Update(MqttReceivedEventArgs* t)
    {
        long long timeStamp = DateTime::Now().Milliseconds();
        string imageResultsValue = JsonFormatter::GetClass(t->Message, "ImageResults");
        vector<string> vehicles = JsonFormatter::GetArray(imageResultsValue, "Vehicles");
        for (std::vector<string>::iterator it = vehicles.begin(); it != vehicles.end(); ++it)
        {
            string id;
            JsonFormatter::Deserialize(*it, tuple<string, string*>("GUID", &id));
            string detectValue = JsonFormatter::GetClass(*it, "Detect");
            string bodyValue = JsonFormatter::GetClass(detectValue, "Body");
            vector<int> rec;
            JsonFormatter::Deserialize(bodyValue, tuple<string, vector<int>*>("Rect", &rec));
            CarItem item(id, timeStamp, rec[0], rec[1], rec[2], rec[3]);
            _speed.Collect(item);
        }
    }

protected:

    void StartCore()
    {
        while (!Cancelled())
        {
            this_thread::sleep_for(chrono::minutes(1));
            double d = _speed.Calculate();
            cout << d << endl;
        }
    }

private:

    Speed _speed;
};

int main()
{
    MqttChannel channel("192.168.201.139", 1884);
    Test t;
    channel.MqttReceived.Subscribe(&t);
    channel.Start();
    t.Start();

    system("pause");
    channel.Stop();
    t.Stop();
    system("pause");

    return 0;
}

//int main()
//{
//    DateTime logDate(2020, 2, 23);
//    Speed speed;
//    std::tuple<std::vector<LogItem>, int> v = LogReader::ReadLogs(LogPool::Directory(), "System", logDate, 0, 0, 1, 1600, false);
//    int i = 0;
//    for (std::vector<LogItem>::iterator it = std::get<0>(v).begin(); it != std::get<0>(v).end(); ++it)
//    {
//        cout << ++i << endl;
//        string s2 = JsonFormatter::GetClass(it->Content, "ImageResults");
//        int timeStamp=0;
//        JsonFormatter::Deserialize(s2, tuple<string, int*>("Timestamp", &timeStamp));
//        vector<string> v = JsonFormatter::GetArray(s2, "Vehicles");
//        for (std::vector<string>::iterator sit = v.begin(); sit != v.end(); ++sit)
//        {
//            string id;
//            JsonFormatter::Deserialize(*sit, tuple<string, string*>("GUID", &id));
//            string s3 = JsonFormatter::GetClass(*sit, "Detect");
//            string s4 = JsonFormatter::GetClass(s3, "Body");
//            vector<int> rec;
//            JsonFormatter::Deserialize(s4, tuple<string, vector<int>*>("Rect", &rec));
//            CarItem item(id,timeStamp,rec[0], rec[1], rec[2], rec[3]);
//
//            speed.Test(item);
//          
//        }
//        //std::string pattern = "\"" + key + "\":";
//        //size_t start = it->Content.find(pattern);
//        //if (start != std::string::npos)
//        //{
//        //    const std::string Head = "{[";
//        //    size_t start1 = it->Content.find_first_of(Head, start);
//        //    int headCount = 0;
//        //    int tailCount = 0;
//        //    size_t end1 = string::npos;
//        //    for (string::iterator it1 = it->Content.begin() + start1; it1 != it->Content.end(); ++it1)
//        //    {
//        //  
//        //        if (*it1 == '[')
//        //        {
//        //            headCount += 1;
//        //        }
//        //        else if (*it1 == ']')
//        //        {
//        //            tailCount += 1;
//        //            if (headCount == tailCount)
//        //            {
//        //                end1 = it1 - it->Content.begin();
//        //                break;
//        //            }
//        //        }
//
//        //    }
//        //    string s1 = it->Content.substr(start1, end1 - start1+1);
//        //    cout <<s1 << endl;
//        //}
//
//
//        //Item item;
//        //string imageResultItem;
//        //JsonFormatter::Deserialize(it->Content, std::tuple<std::string, string*>("ImageResults", &imageResultItem));
//        //cout << item.Code << endl;
//    }
//    double d = speed.Test1();
//    system("pause");
//    return 0;
//}
