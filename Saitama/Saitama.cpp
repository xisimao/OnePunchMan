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



int main()
{
    SocketMaid maid(1);
    DataChannel channel("192.168.201.139", 1884);
    HttpHandler handler;
    handler.HttpReceived.Subscribe(&channel);
    maid.AddListenEndPoint(EndPoint(7778), &handler);
    channel.Start();
    maid.Start();

    system("pause");

    maid.Stop();
    channel.Stop();
    //FlowChannelData data;

//FlowChannel newChannel;
//newChannel.ChannelId = "cid";
//newChannel.ChannelName = "cname";
//newChannel.ChannelIndex = 1;
//newChannel.ChannelType = 2;
//newChannel.RtspUser = "user";
//newChannel.RtspPassword = "pwd";
//newChannel.RtspProtocol = 3;
//newChannel.IsLoop = true;

//FlowLane newLane1;
//newLane1.ChannelId = "cid";
//newLane1.LaneId = "lid";
//newLane1.LaneName = "lname";
//newLane1.LaneIndex = 1;
//newLane1.LaneType = 2;
//newLane1.Direction = 3;
//newLane1.FlowDirection = 4;
//newLane1.Length = 5;
//newLane1.IOIp = "ip";
//newLane1.IOPort = 6;
//newLane1.IOIndex = 7;
//newLane1.DetectLine = "d";
//newLane1.StopLine = "s";
//newLane1.LaneLine1 = "l1";
//newLane1.LaneLine2 = "l2";

//newChannel.Lanes.push_back(newLane1);

//data.Set(newChannel);

//vector<FlowChannel> channels = data.GetList();
//FlowChannel channel = data.Get("cid");
//data.Delete("cid");

    //DateTime logDate(2020, 3, 9);
    //std::tuple<std::vector<LogItem>, int> v = LogReader::ReadLogs(LogPool::Directory(), "System", logDate, 0, 0, 1, 1, false);
    //for (std::vector<LogItem>::iterator it = std::get<0>(v).begin(); it != std::get<0>(v).end(); ++it)
    //{
    //    string json = it->Content;
    //    vector<string> imageJsons = JsonFormatter::DeserializeJsons(json, "ImageResults");
    //    vector<string> vehicleJsons = JsonFormatter::DeserializeJsons(imageJsons[0], "Vehicles");

    //    string detectJson = JsonFormatter::DeserializeJson(vehicleJsons[0], "Detect");
    //    string bodyJson = JsonFormatter::DeserializeJson(detectJson, "Body");
    //    vector<string> rect;
    //    JsonFormatter::Deserialize(bodyJson, "Rect", &rect);
    // }

   

    //DataChannel channel("192.168.201.139", 1884);
    //channel.Start();
    //system("pause");
    //channel.Stop();
    return 0;
}

