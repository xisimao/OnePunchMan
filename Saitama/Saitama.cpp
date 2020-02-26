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
#include "Lane.h"
#include "MqttChannel.h"

using namespace std;
using namespace Saitama;


class Test :public ThreadObject,public IObserver<MqttReceivedEventArgs>
{
public:

    Test(Lane* speed)
        :ThreadObject("test"),_speed(speed)
    {
        _lastMinute = DateTime::Now().Minute();
    }

    void Update(MqttReceivedEventArgs* t)
    {
       
        string imageResultsValue = JsonFormatter::GetClass(t->Message, "ImageResults");
        int videoChannelNo = -1;
        JsonFormatter::Deserialize(t->Message, tuple<string, int*>("VideoChannel", &videoChannelNo));
        _speed->Collect(t->Message);
    }

protected:

    void StartCore()
    {
        while (!Cancelled())
        {
            this_thread::sleep_for(chrono::seconds(1));
            int currentMinute = DateTime::Now().Minute();
            if (currentMinute != _lastMinute)
            {
                _lastMinute = currentMinute;
                _speed->Calculate();
            }
        }
    }

private:

    Lane* _speed;

    int _lastMinute;
};



int main()
{
    //DateTime logDate(2020, 2, 23);
    //std::tuple<std::vector<LogItem>, int> v = LogReader::ReadLogs(LogPool::Directory(), "System", logDate, 0, 0, 1, 1600, false);
    //for (std::vector<LogItem>::iterator it = std::get<0>(v).begin(); it != std::get<0>(v).end(); ++it)
    //{
    //    string s2 = JsonFormatter::GetClass(it->Content, "ImageResults");
    //    int timeStamp = 0;
    //    JsonFormatter::Deserialize(s2, tuple<string, int*>("Timestamp", &timeStamp));
    //    vector<string> v = JsonFormatter::GetArray(s2, "Pedestrains");
    //    if (!v.empty())
    //    {
    //        cout << "123" << endl;
    //    }
    //}
    vector<Point> points;
    points.push_back(Point(680,432));
    points.push_back(Point(954,449));
    points.push_back(Point(915,957));
    points.push_back(Point(494,909));
    points.push_back(Point(683,432));
    Polygon poly(points);
    Lane speed(poly);


    MqttChannel channel("192.168.201.139", 1884);
    Test t(&speed);
    channel.MqttReceived.Subscribe(&t);
    channel.Start();
    t.Start();

    system("pause");
    channel.Stop();
    t.Stop();
    system("pause");

    return 0;
}

