#include "DataChannel.h"

using namespace std;
using namespace Saitama;

const string DataChannel::DetectTopic("aiservice/level_1_imageresult");
const string DataChannel::RecognizeTopic("aiservice/level_2_result");
const string DataChannel::FlowTopic("analysis2manager_crossingflows");
const string DataChannel::VideoTopic("analysis2manager_videostruct");
const string DataChannel::IOTopic("analysis2manager_deviceIO");

DataChannel::DataChannel(const std::string& ip, int port)
    :ThreadObject("data"), _mqtt(NULL),_ip(ip),_port(port)
{
    _lastMinute = DateTime::Now().Minute();
}

void DataChannel::StartCore()
{
    //初始化通道
    for (int i = 0; i < 8; ++i)
    {
        VideoDetector* channel = new VideoDetector();
        channel->Index = i;
        _channels.push_back(channel);
    }
    FlowChannelData data;
    vector<FlowChannel> channels = data.GetList();
    for (vector<FlowChannel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        UpdateChannel(*it);
    }

    //初始化mqtt
    vector<string> topics;
    topics.push_back(DetectTopic);
    topics.push_back(RecognizeTopic);
    _mqtt = new MqttChannel(_ip, _port, topics);
    _mqtt->MqttReceived.Subscribe(this);
    _mqtt->Start();

    while (!Cancelled())
    {
        this_thread::sleep_for(chrono::seconds(1));

        //收集流量
        DateTime now = DateTime::Now();
        int currentMinute = now.Minute();
        if (currentMinute != _lastMinute)
        {
            _lastMinute = currentMinute;
            CollectFlow(now);
        }
    }

    _mqtt->Stop();
    delete _mqtt;
    for (vector<VideoDetector*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        delete (*it);
    }
}

void DataChannel::UpdateChannel(const FlowChannel& channel)
{
    if (channel.ChannelIndex >= 1 && channel.ChannelIndex <= _channels.size())
    {
        vector<LaneDetector*> laneDetectors;
        for (vector<Lane>::const_iterator lit = channel.Lanes.begin(); lit != channel.Lanes.end(); ++lit)
        {
            //vector<int> detectPoints;
            //JsonFormatter::ConvertToValue(lit->DetectLine, &detectPoints, 0);
            //vector<int> stopPoints;
            //JsonFormatter::ConvertToValue(lit->StopLine, &stopPoints, 0);
            //vector<int> laneLinePoints1;
            //JsonFormatter::ConvertToValue(lit->LaneLine1, &laneLinePoints1, 0);
            //vector<int> laneLinePoints2;
            //JsonFormatter::ConvertToValue(lit->LaneLine2, &laneLinePoints2, 0);
            //if (detectPoints.size() >= 4
            //    && stopPoints.size() >= 4
            //    && laneLinePoints1.size() >= 4
            //    && laneLinePoints2.size() >= 4)
            //{
            //    Line detectLine(Point(detectPoints[0], detectPoints[1]), Point(detectPoints[2], detectPoints[3]));
            //    Line stopLine(Point(stopPoints[0], stopPoints[1]), Point(stopPoints[2], stopPoints[3]));
            //    Line laneLine1(Point(laneLinePoints1[0], laneLinePoints1[1]), Point(laneLinePoints1[2], laneLinePoints1[3]));
            //    Line laneLine2(Point(laneLinePoints2[0], laneLinePoints2[1]), Point(laneLinePoints2[2], laneLinePoints2[3]));
            //    vector<Point> polygonPoints;
            //    polygonPoints.push_back(detectLine.Intersect(laneLine1));
            //    polygonPoints.push_back(laneLine1.Intersect(stopLine));
            //    polygonPoints.push_back(stopLine.Intersect(laneLine2));
            //    polygonPoints.push_back(laneLine2.Intersect(detectLine));
            //    laneDetectors.push_back(new LaneDetector(lit->LaneId, lit->LaneIndex, Polygon(polygonPoints)));
            //}

            vector<int> points;
            JsonFormatter::ConvertToValue(lit->Region, &points, 0);
            vector<Point> polygonPoints;
            if (points.size() > 8)
            {
        
                polygonPoints.push_back(Point(points[0], points[1]));
                polygonPoints.push_back(Point(points[2], points[3]));
                polygonPoints.push_back(Point(points[4], points[5]));
                polygonPoints.push_back(Point(points[6], points[7]));
                polygonPoints.push_back(Point(points[8], points[9]));
                
            }
            laneDetectors.push_back(new LaneDetector(lit->LaneId, lit->LaneIndex, Polygon(polygonPoints)));
          
        }
        _channels[channel.ChannelIndex-1]->Id = channel.ChannelId;
        _channels[channel.ChannelIndex-1]->UpdateLanes(laneDetectors);
    }
}

void DataChannel::Update(MqttReceivedEventArgs* e)
{
    if (e->Topic.compare(DetectTopic) == 0)
    {
        HandleDetect(e->Message);
    }
    else if (e->Topic.compare(RecognizeTopic) == 0)
    {
        HandleRecognize(e->Message);
    }

}

vector<DetectItem> DataChannel::GetDetectItems(const string& json, const string& key, long long timeStamp)
{
    vector<string> values = JsonFormatter::DeserializeJsons(json, key);
    vector<DetectItem> items;
    for (vector<string>::iterator it = values.begin(); it != values.end(); ++it)
    {
        string id;
        JsonFormatter::Deserialize(*it, "GUID", &id);
        int type;
        JsonFormatter::Deserialize(*it, "Type", &type);
        string detect=JsonFormatter::DeserializeJson(*it, "Detect");
        string body=JsonFormatter::DeserializeJson(detect, "Body");
        vector<int> rect;
        JsonFormatter::Deserialize(body, "Rect", &rect);
        if (rect.size() >= 4)
        {
            items.push_back(DetectItem(Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]), id, timeStamp, type));
        }
    }
    return items;
}

DetectItem DataChannel::GetDetectItem(const string& json)
{
    DetectItem item;
    string detectJson = JsonFormatter::DeserializeJson(json, "Detect");
    string bodyJson = JsonFormatter::DeserializeJson(detectJson, "Body");
    vector<int> rect;
    JsonFormatter::Deserialize(bodyJson, "Rect", &rect);
    return DetectItem(Rectangle(Point(rect[0], rect[1]), rect[2], rect[3]));
}

void DataChannel::HandleDetect(const string& json)
{
    int channelIndex = -1;
    JsonFormatter::Deserialize(json, "VideoChannel", &channelIndex);
    if (channelIndex >= 0 && channelIndex < _channels.size())
    {
        long long timeStamp = DateTime::Now().Milliseconds();
        vector<DetectItem> vehicles = GetDetectItems(json, "Vehicles", timeStamp);
        vector<DetectItem> bikes = GetDetectItems(json, "Bikes", timeStamp);
        vector<DetectItem> pedestrains = GetDetectItems(json, "Pedestrains", timeStamp);


        vector<IOItem> items = _channels[channelIndex]->Detect(vehicles, bikes, pedestrains);

        for (vector<IOItem>::iterator it = items.begin(); it != items.end(); ++it)
        {
       
            string laneJson;
            JsonFormatter::Serialize(&laneJson, "laneId", it->LaneId);
            JsonFormatter::Serialize(&laneJson, "laneIndex", it->LaneIndex);
            JsonFormatter::Serialize(&laneJson, "status", it->Status ? 1 : 0);
            JsonFormatter::Serialize(&laneJson, "type", (int)DetectType::Car);
            string lanesJson;
            JsonFormatter::SerializeItem(&lanesJson, laneJson);

            string channelJson;
            JsonFormatter::Serialize(&channelJson, "channelId", it->ChannelIndex);
            JsonFormatter::Serialize(&channelJson, "channelReadlId", it->ChannelId);
           
            string channelsJson;
            JsonFormatter::SerializeItem(&channelsJson, channelJson);

            string ioJson;
            JsonFormatter::Serialize(&ioJson, "timestamp", DateTime::Now().Milliseconds());
            JsonFormatter::SerializeJson(&ioJson, "detail", channelsJson);
            LogPool::Debug("channel:", it->ChannelIndex, "lane:", it->LaneIndex, "io:", it->Status);
            _mqtt->Send(IOTopic, ioJson, false);
        }
    }
}

void DataChannel::HandleRecognize(const string& json)
{
    long long timeStamp = DateTime::Now().Milliseconds();
    vector<string> imageJsons = JsonFormatter::DeserializeJsons(json, "ImageResults");
    vector<string> detectJsons = JsonFormatter::DeserializeJsons(json, "l1_result");
    vector<string> images;
    JsonFormatter::Deserialize(json, "imgdata_result",&images);

    for (int i=0;i<imageJsons.size();++i)
    {
        if (i < detectJsons.size())
        {
            vector<string> vehicleJsons = JsonFormatter::DeserializeJsons(imageJsons[i], "Vehicles");
            if (!vehicleJsons.empty())
            {
                int channelIndex = -1;
                JsonFormatter::Deserialize(detectJsons[i], "VideoChannel", &channelIndex);
                if (channelIndex >= 0 && channelIndex < _channels.size())
                {
                    DetectItem item = GetDetectItem(detectJsons[i]);
                    string laneId = _channels[channelIndex]->Contains(item);
                    if (!laneId.empty())
                    {
                        VideoVehicle vehicle;
                        JsonFormatter::Deserialize(vehicleJsons[0], "Type", &vehicle.CarType);

                        string recognizeJson = JsonFormatter::DeserializeJson(vehicleJsons[0], "Recognize");
                        string featureJson = JsonFormatter::DeserializeJson(recognizeJson, "Feature");
                        JsonFormatter::Deserialize(featureJson, "Feature",&vehicle.Feature);

                        string colorJson = JsonFormatter::DeserializeJson(recognizeJson, "Color");
                        vector<string> topListJson = JsonFormatter::DeserializeJsons(colorJson, "TopList");
                        if (!topListJson.empty())
                        {
                            string color;
                            JsonFormatter::Deserialize(topListJson[0], "Code", &color);
                            StringEx::Convert(color,&vehicle.CarColor);
                        }

                        string typeJson = JsonFormatter::DeserializeJson(recognizeJson, "Type");
                        topListJson = JsonFormatter::DeserializeJsons(colorJson, "TopList");
                        if (!topListJson.empty())
                        {
                            string type;
                            JsonFormatter::Deserialize(topListJson[0], "Code", &type);
                            StringEx::Convert(type, &vehicle.CarType);
                        }

                        string brandJson = JsonFormatter::DeserializeJson(recognizeJson, "Brand");
                        topListJson = JsonFormatter::DeserializeJsons(colorJson, "TopList");
                        if (!topListJson.empty())
                        {
                            JsonFormatter::Deserialize(topListJson[0], "Name", &vehicle.CarBrand);
                        }

                        string plateJson = JsonFormatter::DeserializeJson(recognizeJson, "Plate");
                        JsonFormatter::Deserialize(plateJson, "Type", &vehicle.PlateType);
                        JsonFormatter::Deserialize(plateJson, "Licence", &vehicle.PlateNumber);

                        if (i < imageJsons.size())
                        {
                            vehicle.Image = images[i];
                        }

                        string sendJson;
                        JsonFormatter::Serialize(&sendJson, "ChannelId", _channels[channelIndex]->Id);
                        JsonFormatter::Serialize(&sendJson, "LaneId", laneId);
                        JsonFormatter::Serialize(&sendJson, "TimeStamp", timeStamp);
                        JsonFormatter::Serialize(&sendJson, "Feature", vehicle.Feature);
                        JsonFormatter::Serialize(&sendJson, "Image", vehicle.Image);
                        JsonFormatter::Serialize(&sendJson, "VideoStructType", vehicle.VideoStructType);
                        JsonFormatter::Serialize(&sendJson, "CarType", vehicle.CarType);
                        JsonFormatter::Serialize(&sendJson, "CarColor", vehicle.CarColor);
                        JsonFormatter::Serialize(&sendJson, "CarBrand", vehicle.CarBrand);
                        JsonFormatter::Serialize(&sendJson, "PlateType", vehicle.PlateType);
                        JsonFormatter::Serialize(&sendJson, "PlateNumber", vehicle.PlateNumber);
                        _mqtt->Send(VideoTopic, sendJson, false);
                    }
                }
                continue;
            }
            vector<string> bikeJsons = JsonFormatter::DeserializeJsons(imageJsons[i], "Bikes");
            if (!bikeJsons.empty())
            {
                int channelIndex = -1;
                JsonFormatter::Deserialize(detectJsons[i], "VideoChannel", &channelIndex);
                if (channelIndex >= 0 && channelIndex < _channels.size())
                {
                    DetectItem item = GetDetectItem(vehicleJsons[0]);
                    string laneId = _channels[channelIndex]->Contains(item);
                    if (!laneId.empty())
                    {
                        VideoBike bike;
                        JsonFormatter::Deserialize(vehicleJsons[0], "Type", &bike.BikeType);

                        string recognizeJson = JsonFormatter::DeserializeJson(vehicleJsons[0], "Recognize");
                        string featureJson = JsonFormatter::DeserializeJson(recognizeJson, "Feature");
                        bike.Feature = JsonFormatter::DeserializeJson(featureJson, "Feature");

                        if (i < imageJsons.size())
                        {
                            bike.Image = images[i];
                        }

                        string sendJson;
                        JsonFormatter::Serialize(&sendJson, "ChannelId", _channels[channelIndex]->Id);
                        JsonFormatter::Serialize(&sendJson, "LaneId", laneId);
                        JsonFormatter::Serialize(&sendJson, "TimeStamp", timeStamp);
                        JsonFormatter::Serialize(&sendJson, "Feature", bike.Feature);
                        JsonFormatter::Serialize(&sendJson, "Image", bike.Image);
                        JsonFormatter::Serialize(&sendJson, "VideoStructType", bike.VideoStructType);
                        JsonFormatter::Serialize(&sendJson, "BikeType", bike.BikeType);
                        _mqtt->Send(VideoTopic, sendJson, false);
                    }
                }
                continue;
            }
            vector<string> pedestrainJsons = JsonFormatter::DeserializeJsons(imageJsons[i], "Pedestrains");
            if (!pedestrainJsons.empty())
            {
                int channelIndex = -1;
                JsonFormatter::Deserialize(detectJsons[i], "VideoChannel", &channelIndex);
                if (channelIndex >= 0 && channelIndex < _channels.size())
                {
                    DetectItem item = GetDetectItem(vehicleJsons[0]);
                    string laneId = _channels[channelIndex]->Contains(item);
                    if (!laneId.empty())
                    {
                        VideoPedestrain pedestrain;
                        string recognizeJson = JsonFormatter::DeserializeJson(vehicleJsons[0], "Recognize");
                        string featureJson = JsonFormatter::DeserializeJson(recognizeJson, "Feature");
                        pedestrain.Feature = JsonFormatter::DeserializeJson(featureJson, "Feature");
                        string sexJson = JsonFormatter::DeserializeJson(recognizeJson, "Sex");
                        vector<string> topListJson = JsonFormatter::DeserializeJsons(sexJson, "TopList");
                        if (!topListJson.empty())
                        {
                            JsonFormatter::Deserialize(topListJson[0], "Code", &pedestrain.Sex);
                        }
                        string ageJson = JsonFormatter::DeserializeJson(recognizeJson, "Age");
                        topListJson = JsonFormatter::DeserializeJsons(sexJson, "TopList");
                        if (!topListJson.empty())
                        {
                            JsonFormatter::Deserialize(topListJson[0], "Code", &pedestrain.Age);
                        }
                        string upperColorJson = JsonFormatter::DeserializeJson(recognizeJson, "UpperColor");
                        topListJson = JsonFormatter::DeserializeJsons(upperColorJson, "TopList");
                        if (!topListJson.empty())
                        {
                            JsonFormatter::Deserialize(topListJson[0], "Code", &pedestrain.UpperColor);
                        }
                        if (i < imageJsons.size())
                        {
                            pedestrain.Image = images[i];
                        }

                        string sendJson;
                        JsonFormatter::Serialize(&sendJson, "ChannelId", _channels[channelIndex]->Id);
                        JsonFormatter::Serialize(&sendJson, "LaneId", laneId);
                        JsonFormatter::Serialize(&sendJson, "TimeStamp", timeStamp);
                        JsonFormatter::Serialize(&sendJson, "Feature", pedestrain.Feature);
                        JsonFormatter::Serialize(&sendJson, "Image", pedestrain.Image);
                        JsonFormatter::Serialize(&sendJson, "VideoStructType", pedestrain.VideoStructType);
                        JsonFormatter::Serialize(&sendJson, "Sex", pedestrain.Sex);
                        JsonFormatter::Serialize(&sendJson, "Age", pedestrain.Age);
                        JsonFormatter::Serialize(&sendJson, "UpperColor", pedestrain.UpperColor);
                        _mqtt->Send(VideoTopic, sendJson, false);
                    }
                }
                continue;
            }
        }
        
    }
}

void DataChannel::CollectFlow(const DateTime& now)
{
    long long timeStamp = DateTime(now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), 0).Milliseconds();
    for (vector<VideoDetector*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        vector<LaneItem> lanes = (*it)->Collect();
        if (!lanes.empty())
        {
            string channelJson;
            JsonFormatter::Serialize(&channelJson, "channelId", (*it)->Index);
            JsonFormatter::Serialize(&channelJson, "channelRealId", (*it)->Id);
            JsonFormatter::Serialize(&channelJson, "timestamp", timeStamp);

            string lanesJson;
            for (vector<LaneItem>::iterator lit = lanes.begin(); lit != lanes.end(); ++lit)
            {
                LogPool::Debug("channel:", (*it)->Index, "lane:", lit->Index, "cars:", lit->Cars + lit->Tricycles + lit->Buss + lit->Vans + lit->Trucks, "bikes:", lit->Bikes + lit->Motorcycles, "persons:", lit->Persons, lit->Speed, "km/h ", lit->HeadDistance, "sec ", lit->TimeOccupancy, "%");

                string laneJson;

                JsonFormatter::Serialize(&laneJson, "crossingId", StringEx::ToString(lit->Index));
                JsonFormatter::Serialize(&laneJson, "laneId", lit->Id);

                JsonFormatter::Serialize(&laneJson, "persons", lit->Persons);
                JsonFormatter::Serialize(&laneJson, "bikes", lit->Bikes);
                JsonFormatter::Serialize(&laneJson, "motorcycles", lit->Motorcycles);
                JsonFormatter::Serialize(&laneJson, "cars", lit->Cars);
                JsonFormatter::Serialize(&laneJson, "tricycles", lit->Tricycles);
                JsonFormatter::Serialize(&laneJson, "buss", lit->Buss);
                JsonFormatter::Serialize(&laneJson, "vans", lit->Vans);
                JsonFormatter::Serialize(&laneJson, "trucks", lit->Trucks);

                JsonFormatter::Serialize(&laneJson, "averageSpeed", lit->Speed);
                JsonFormatter::Serialize(&laneJson, "headDistance", lit->HeadDistance);
                JsonFormatter::Serialize(&laneJson, "timeOccupancy", lit->TimeOccupancy);
                JsonFormatter::SerializeItem(&lanesJson, laneJson);
            }
            JsonFormatter::SerializeJson(&channelJson, "crossingData", lanesJson);

            _mqtt->Send(FlowTopic, channelJson);
        }
    }
}

void DataChannel::Update(HttpReceivedEventArgs* e)
{
    string channelUrl("/api/channels");
    if (UrlStartWith(e->Url, channelUrl))
    {
        FlowChannelData data;
        if (e->Function.compare(HttpFunction::Get) == 0)
        {
            e->Code = HttpCode::OK;
            vector<FlowChannel> channels = data.GetList();
            string channelsJson;
            for (vector<FlowChannel>::iterator cit = channels.begin(); cit != channels.end(); ++cit)
            {
                string channelJson;
                JsonFormatter::Serialize(&channelJson, "channelId", cit->ChannelId);
                JsonFormatter::Serialize(&channelJson, "channelName", cit->ChannelName);
                JsonFormatter::Serialize(&channelJson, "channelIndex", cit->ChannelIndex);
                JsonFormatter::Serialize(&channelJson, "channelType", cit->ChannelType);
                JsonFormatter::Serialize(&channelJson, "rtspUser", cit->RtspUser);
                JsonFormatter::Serialize(&channelJson, "rtspPassword", cit->RtspPassword);
                JsonFormatter::Serialize(&channelJson, "rtspProtocol", cit->RtspProtocol);
                JsonFormatter::Serialize(&channelJson, "isLoop", cit->IsLoop);

                string lanesJson;
                for (vector<Lane>::const_iterator lit = cit->Lanes.begin(); lit != cit->Lanes.end(); ++lit)
                {
                    string laneJson;
                    JsonFormatter::Serialize(&laneJson, "channelId", lit->ChannelId);
                    JsonFormatter::Serialize(&laneJson, "laneId", lit->LaneId);
                    JsonFormatter::Serialize(&laneJson, "laneName", lit->LaneName);
                    JsonFormatter::Serialize(&laneJson, "laneIndex", lit->LaneIndex);
                    JsonFormatter::Serialize(&laneJson, "laneType", lit->LaneType);
                    JsonFormatter::Serialize(&laneJson, "direction", lit->Direction);
                    JsonFormatter::Serialize(&laneJson, "flowDirection", lit->FlowDirection);
                    JsonFormatter::Serialize(&laneJson, "length", lit->Length);
                    JsonFormatter::Serialize(&laneJson, "iOIp", lit->IOIp);
                    JsonFormatter::Serialize(&laneJson, "iOPort", lit->IOPort);
                    JsonFormatter::Serialize(&laneJson, "iOIndex", lit->IOIndex);
                    JsonFormatter::Serialize(&laneJson, "detectLine", lit->DetectLine);
                    JsonFormatter::Serialize(&laneJson, "stopLine", lit->StopLine);
                    JsonFormatter::Serialize(&laneJson, "laneLine1", lit->LaneLine1);
                    JsonFormatter::Serialize(&laneJson, "laneLine2", lit->LaneLine2);
                    JsonFormatter::Serialize(&laneJson, "region", lit->Region);
                    JsonFormatter::SerializeItem(&lanesJson, laneJson);
                }
                JsonFormatter::SerializeJson(&channelJson, "lanes", lanesJson);

                JsonFormatter::SerializeItem(&channelsJson, channelJson);
            }

            e->ResponseJson = channelsJson;
        }
        else if (e->Function.compare(HttpFunction::Post) == 0)
        {
            FlowChannel channel;
            JsonFormatter::Deserialize(e->RequestJson, "channelId", &channel.ChannelId);
            JsonFormatter::Deserialize(e->RequestJson, "channelName", &channel.ChannelName);
            JsonFormatter::Deserialize(e->RequestJson, "channelIndex", &channel.ChannelIndex);
            JsonFormatter::Deserialize(e->RequestJson, "channelType", &channel.ChannelType);
            JsonFormatter::Deserialize(e->RequestJson, "rtspUser", &channel.RtspUser);
            JsonFormatter::Deserialize(e->RequestJson, "rtspPassword", &channel.RtspPassword);
            JsonFormatter::Deserialize(e->RequestJson, "rtspProtocol", &channel.RtspProtocol);
            JsonFormatter::Deserialize(e->RequestJson, "isLoop", &channel.IsLoop);

            vector<string> laneJsons = JsonFormatter::DeserializeJsons(e->RequestJson, "lanes");
            for (vector<string>::iterator it = laneJsons.begin(); it != laneJsons.end(); ++it)
            {
                Lane lane;
                lane.ChannelId = channel.ChannelId;
                JsonFormatter::Deserialize(*it, "laneId", &lane.LaneId);
                JsonFormatter::Deserialize(*it, "laneName", &lane.LaneName);
                JsonFormatter::Deserialize(*it, "laneIndex", &lane.LaneIndex);
                JsonFormatter::Deserialize(*it, "laneType", &lane.LaneType);
                JsonFormatter::Deserialize(*it, "direction", &lane.Direction);
                JsonFormatter::Deserialize(*it, "flowDirection", &lane.FlowDirection);
                JsonFormatter::Deserialize(*it, "length", &lane.Length);
                JsonFormatter::Deserialize(*it, "iOIp", &lane.IOIp);
                JsonFormatter::Deserialize(*it, "iOPort", &lane.IOPort);
                JsonFormatter::Deserialize(*it, "iOIndex", &lane.IOIndex);
                JsonFormatter::Deserialize(*it, "detectLine", &lane.DetectLine);
                JsonFormatter::Deserialize(*it, "stopLine", &lane.StopLine);
                JsonFormatter::Deserialize(*it, "laneLine1", &lane.LaneLine1);
                JsonFormatter::Deserialize(*it, "laneLine2", &lane.LaneLine2);
                JsonFormatter::Deserialize(*it, "region", &lane.Region);
                channel.Lanes.push_back(lane);
            }
            if (data.Set(channel))
            {
                UpdateChannel(channel);
                e->Code = HttpCode::OK;
            }
            else
            {
                e->Code = HttpCode::InternalServerError;
            }
        }
        else if (e->Function.compare(HttpFunction::Delete) == 0)
        {
            string channelId = GetId(e->Url, channelUrl);
            int result = data.Delete(channelId);
            if (result == 1)
            {
                e->Code = HttpCode::OK;
            }
            else if (result == 0)
            {
                e->Code = HttpCode::NotFound;
            }
            else
            {
                e->Code = HttpCode::InternalServerError;
            }
        }
    }

}

bool DataChannel::UrlStartWith(const string& url, const string& key)
{
    return url.size() >= key.size() && url.substr(0, key.size()).compare(key) == 0;
}

string DataChannel::GetId(const string& url, const string& key)
{
    return url.substr(key.size() + 1, url.size() - key.size() + 1);
}


