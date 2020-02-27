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

using namespace std;
using namespace Saitama;

int main()
{
   /* DateTime logDate(2020, 2, 23);
    std::tuple<std::vector<LogItem>, int> v = LogReader::ReadLogs(LogPool::Directory(), "System", logDate, 0, 0, 1, 1, false);
    for (std::vector<LogItem>::iterator it = std::get<0>(v).begin(); it != std::get<0>(v).end(); ++it)
    {
        int channelIndex = -1;
        JsonFormatter::Deserialize(it->Content, "ChannelIndex", &channelIndex);
        vector<string> lanes;
        JsonFormatter::Deserialize(it->Content, "CrossingFlows", &lanes);
        vector<Polygon> polygons;
        for (vector<string>::iterator it = lanes.begin(); it != lanes.end(); ++it)
        {
            vector<string> pointPairs;
            JsonFormatter::Deserialize(*it, "region", &pointPairs);
            vector<Point> points;

            for (vector<string>::iterator pit = pointPairs.begin();pit != pointPairs.end(); ++pit)
            {
                vector<int> values;
                JsonFormatter::DeserializeValue(*pit, &values);
                if (values.size() == 2)
                {
                    points.push_back(Point(values[0], values[1]));
                }
            }
            polygons.push_back(Polygon(points));
        }
        
     }*/

    vector<int> is;
    is.push_back(1);
    is.push_back(2);
    is.push_back(3);

    set<int> iss;
    iss.insert(1);
    iss.insert(2);
    iss.insert(3);
    string json;
    string aa("aaa");
    JsonFormatter::Serialize(&json, "a", 123);
    JsonFormatter::Serialize(&json, "b", aa);

    //DataChannel channel("192.168.201.139", 1884);
    //channel.Start();
    //system("pause");
    //channel.Stop();
    return 0;
}

