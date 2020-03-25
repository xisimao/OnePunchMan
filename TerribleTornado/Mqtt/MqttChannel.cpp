#include "MqttChannel.h"

using namespace std;
using namespace Saitama;

const int MqttChannel::KeepAlive = 60;
const int MqttChannel::ConnectSpan = 5000;
const int MqttChannel::PollTime = 100;

MqttChannel::MqttChannel(const string& ip, int port, std::vector<std::string> topics)
    :ThreadObject("mqtt"), _mosq(NULL), _ip(ip), _port(port), _topics(topics), _status(-1)
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

void MqttChannel::SubscribedEventHandler(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos)
{
    LogPool::Information(LogEvent::Mqtt, "mqtt subscribe success", mid, qos_count);
}

void MqttChannel::ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
    MqttReceivedEventArgs args(message->topic,(const char*)message->payload);
    static_cast<MqttChannel*>(userdata)->MqttReceived.Notice(&args);
}

bool MqttChannel::Send(const string& topic, const string& message, bool lock)
{
    if (_status==0)
    {
        if (lock)
        {
            lock_guard<mutex> lck(_mutex);
            return mosquitto_publish(_mosq, NULL, topic.c_str(), static_cast<int>(message.size()), message.c_str(), 2, 0) == 0;
        }
        else
        {
            return mosquitto_publish(_mosq, NULL, topic.c_str(), static_cast<int>(message.size()), message.c_str(), 2, 0) == 0;
        }
    }
    else
    {
        return false;
    }
    return true;
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
        mosquitto_destroy(_mosq);
        mosquitto_lib_cleanup();
        return;
    }

    mosquitto_connect_callback_set(_mosq, ConnectedEventHandler);
    mosquitto_subscribe_callback_set(_mosq, SubscribedEventHandler);
    mosquitto_message_callback_set(_mosq, ReceivedEventHandler);

    while (!Cancelled())
    {
        unique_lock<mutex> lck(_mutex);
        if (_status == 0)
        {
            int i = mosquitto_loop(_mosq, 0, 1);
            if (i == 4)
            {
                if (_status == 0)
                {
                    _status = -1;
                    LogPool::Information(LogEvent::Mqtt, "mqtt disconnect");
                }
            }
        }
        else
        {
            _status = mosquitto_connect(_mosq, _ip.c_str(), _port, KeepAlive);
        }
        lck.unlock();
        if (_status == 0)
        {
            this_thread::sleep_for(chrono::milliseconds(PollTime));
        }
        else
        {
            this_thread::sleep_for(chrono::milliseconds(ConnectSpan));
        }
    }
    mosquitto_disconnect(_mosq);
    mosquitto_destroy(_mosq);
    mosquitto_lib_cleanup();
}