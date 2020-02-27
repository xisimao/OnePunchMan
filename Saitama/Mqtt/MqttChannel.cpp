#include "MqttChannel.h"

using namespace std;
using namespace Saitama;

const int MqttChannel::KeepAlive = 60;
const int MqttChannel::ConnectSpan = 5000;

MqttChannel::MqttChannel(const string& ip, int port, std::vector<std::string> topics)
    :ThreadObject("mqtt"), _mosq(NULL), _ip(ip), _port(port), _topics(topics)
{

}

void MqttChannel::ConnectedEventHandler(struct mosquitto* mosq, void* userdata, int result)
{
    LogPool::Information(LogEvent::Mqtt, "mqtt connect success",result);
    MqttChannel* channel=static_cast<MqttChannel*>(userdata);
    for (vector<string>::iterator it = channel->_topics.begin(); it != channel->_topics.end(); ++it)
    {
        mosquitto_subscribe(mosq, NULL, it->c_str(), 2);
    }
}

void MqttChannel::ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
    MqttReceivedEventArgs args(message->topic,(const char*)message->payload);
    static_cast<MqttChannel*>(userdata)->MqttReceived.Notice(&args);
}

bool MqttChannel::Send(const string& topic, const string& message)
{
    if (_mosq == NULL)
    {
        return false;
    }
    return mosquitto_publish(_mosq, NULL, topic.c_str(), message.size(), message.c_str(), 2, 0)==0;
}

void MqttChannel::StartCore()
{
    //libmosquitto 库初始化
    mosquitto_lib_init();
    //创建mosquitto客户端
    _mosq = mosquitto_new(NULL, true, this);
    if (_mosq)
    {
        LogPool::Information(LogEvent::Mqtt, "mqtt init success");
    }
    else
    {
        LogPool::Information(LogEvent::Mqtt, "mqtt init failed");
        mosquitto_lib_cleanup();
        return;
    }

    mosquitto_connect_callback_set(_mosq, ConnectedEventHandler);
    mosquitto_message_callback_set(_mosq, ReceivedEventHandler);

    int connectResult = -1;
    while (!Cancelled())
    {
        if (connectResult == 0)
        {
            int i = mosquitto_loop(_mosq, -1, 1);
            if (i == 4)
            {
                if (connectResult == 0)
                {
                    connectResult = -1;
                    LogPool::Information(LogEvent::Mqtt, "mqtt disconnect");
                }
            }
        }
        else
        {
            connectResult = mosquitto_connect(_mosq, _ip.c_str(), _port, KeepAlive);
        }
        if (connectResult != 0)
        {
            this_thread::sleep_for(chrono::milliseconds(ConnectSpan));
        }
    }
    mosquitto_disconnect(_mosq);
    mosquitto_destroy(_mosq);
    mosquitto_lib_cleanup();
}