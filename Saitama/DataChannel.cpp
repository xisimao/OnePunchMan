#include "DataChannel.h"

using namespace std;
using namespace Saitama;

const string DataChannel::DetectTopic("aiservice/level_1_imageresult");
const string DataChannel::ChannelTopic("manager2analysis_crossingparam");
const string DataChannel::ChannelRequestTopic("analysis2manager_crossingparam_request");
const string DataChannel::TrafficTopic("analysis2manager_crossingflows");
const string DataChannel::IOTopic("analysis2manager_deviceIO");

DataChannel::DataChannel(const std::string& ip, int port)
    :ThreadObject("data"), _ip(ip),_port(port)
{
    _lastMinute = DateTime::Now().Minute();
}

void DataChannel::Update(MqttReceivedEventArgs* e)
{
    if (e->Topic[0] == 'a')
    {
        string imageResults;
        JsonFormatter::Deserialize(e->Message, "ImageResults",&imageResults);
        int channelIndex =-1;
        JsonFormatter::Deserialize(e->Message, "VideoChannel", &channelIndex);
        if (channelIndex >= 0 && channelIndex < _videoChannels.size())
        {
            _videoChannels[channelIndex]->Push(e->Message);
        }
    }
    else
    {
        int channelIndex = -1;
        JsonFormatter::Deserialize(e->Message, "ChannelIndex", &channelIndex);
        if (channelIndex >= 0 && channelIndex < _videoChannels.size())
        {
            vector<string> lanesValue;
            JsonFormatter::Deserialize(e->Message, "CrossingFlows", &lanesValue);
            vector<Lane*> lanes;
            for (vector<string>::iterator it = lanesValue.begin(); it != lanesValue.end(); ++it)
            {
                vector<string> pointPairs;
                JsonFormatter::Deserialize(*it, "region", &pointPairs);
                string id;
                JsonFormatter::Deserialize(*it, "id", &id);
                vector<Point> points;

                for (vector<string>::iterator pit = pointPairs.begin(); pit != pointPairs.end(); ++pit)
                {
                    vector<int> values;
                    JsonFormatter::DeserializeValue(*pit, &values);
                    if (values.size() == 2)
                    {
                        points.push_back(Point(values[0], values[1]));
                    }
                }
                lanes.push_back(new Lane(id, Polygon(points)));
            }
            _videoChannels[channelIndex]->UpdateLanes(lanes);
        }
       
    }
}

void DataChannel::StartCore()
{   
    //初始化通道
    for (int i = 0; i < 8; ++i)
    {
        VideoChannel* channel = new VideoChannel();
        _videoChannels.push_back(channel);
        channel->Start();
    }

    //初始化mqtt
    vector<string> topics;
    topics.push_back(DetectTopic);
    topics.push_back(ChannelTopic);
    MqttChannel mqtt(_ip, _port, topics);
    mqtt.MqttReceived.Subscribe(this);
    mqtt.Start();
    this_thread::sleep_for(chrono::seconds(3));
    mqtt.Send(ChannelRequestTopic, "1");

    while (!Cancelled())
    {
        this_thread::sleep_for(chrono::seconds(1));
        int currentMinute = DateTime::Now().Minute();
        if (currentMinute != _lastMinute)
        {
            _lastMinute = currentMinute;
            for (vector<VideoChannel*>::iterator it = _videoChannels.begin(); it != _videoChannels.end(); ++it)
            {
          

            }
        }
    }

    mqtt.Stop();
    for (vector<VideoChannel*>::iterator it = _videoChannels.begin(); it != _videoChannels.end(); ++it)
    {
        (*it)->Stop();
        delete (*it);
    }
}