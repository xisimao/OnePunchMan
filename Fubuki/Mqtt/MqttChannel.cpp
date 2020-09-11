#include "MqttChannel.h"

using namespace std;
using namespace OnePunchMan;

const int MqttChannel::KeepAlive = 10;
const int MqttChannel::ConnectSpan = 5000;
const int MqttChannel::SleepTime = 500;

MqttChannel::MqttChannel(const string& ip, int port)
    :MqttChannel(ip, port, vector<string>())
{

}

MqttChannel::MqttChannel(const string& ip, int port, vector<string> topics)
    : ThreadObject("mqtt"), _mosq(NULL), _ip(ip), _port(port), _topics(topics),_status(MqttStatus::None)
{

}

void MqttChannel::Init()
{
    mosquitto_lib_init();
}

void MqttChannel::Uninit()
{
    mosquitto_lib_cleanup();
}

void MqttChannel::ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
    MqttReceivedEventArgs args(message->topic, (const char*)message->payload);
    static_cast<MqttChannel*>(userdata)->MqttReceived.Notice(&args);
}

MqttStatus MqttChannel::Status()
{
    return _status;
}

bool MqttChannel::Send(const string& topic, const string& message, int qos)
{
    if (_status == MqttStatus::Connected)
    {
        return mosquitto_publish(_mosq, NULL, topic.c_str(), static_cast<int>(message.size()), message.c_str(), qos, 0) == 0;
    }
    else
    {
        return false;
    }

}

bool MqttChannel::Send(const string& topic, const unsigned char* message, unsigned int size, int qos)
{
    if (_status == MqttStatus::Connected)
    {
        return mosquitto_publish(_mosq, NULL, topic.c_str(), size, message, qos, 0) == 0;
    }
    else
    {
        return false;
    }
  
}

void MqttChannel::StartCore()
{
    //创建mosquitto客户端
    _mosq = mosquitto_new(NULL, true, this);
    if (_mosq==NULL)
    {
        LogPool::Error(LogEvent::Mqtt, "mosquitto_new");
        mosquitto_destroy(_mosq);
        mosquitto_lib_cleanup();
        return;
    }
    if (!_topics.empty())
    {
        mosquitto_message_callback_set(_mosq, ReceivedEventHandler);
    }
    _status = mosquitto_connect(_mosq, _ip.c_str(), _port, KeepAlive) == 0
        ? MqttStatus::Connected
        : MqttStatus::Disconnected;
    for (vector<string>::iterator it = _topics.begin(); it != _topics.end(); ++it)
    {
        mosquitto_subscribe(_mosq, NULL, it->c_str(), 0);
    }
    if (_status == MqttStatus::Connected)
    {
        LogPool::Information(LogEvent::Mqtt, "mqtt connect,ip:",_ip.c_str(),"port:",_port);
    }
    while (!_cancelled)
    {
        if (_status == MqttStatus::Connected)
        {
            int i = mosquitto_loop(_mosq, -1, 1);
            if (i == MOSQ_ERR_NO_CONN)
            {
                _status = MqttStatus::Disconnected;
                LogPool::Information(LogEvent::Mqtt, "mqtt disconnect,ip:", _ip.c_str(), "port:", _port);
                MqttDisconnectedEventArgs e;
                MqttDisconnected.Notice(&e);
            }
        }
        else
        {
            _status = mosquitto_reconnect(_mosq) == 0
                ? MqttStatus::Connected
                : MqttStatus::Disconnected;
            if (_status == MqttStatus::Connected)
            {
                for (vector<string>::iterator it = _topics.begin(); it != _topics.end(); ++it)
                {
                    mosquitto_subscribe(_mosq, NULL, it->c_str(), 0);
                }
                LogPool::Information(LogEvent::Mqtt, "mqtt reconnect,ip:", _ip.c_str(), "port:", _port);
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(ConnectSpan));
            }
        }

    }
    mosquitto_disconnect(_mosq);
    mosquitto_destroy(_mosq);
    _status = MqttStatus::Disconnected;
}