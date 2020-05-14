#include "MqttChannel.h"

using namespace std;
using namespace OnePunchMan;

const int MqttChannel::KeepAlive = 10;
const int MqttChannel::ConnectSpan = 5000;
const int MqttChannel::PollTime = 100;

MqttChannel::MqttChannel(const string& ip, int port)
    :ThreadObject("mqtt"), _mosq(NULL), _ip(ip), _port(port), _connected(false)
{

}

bool MqttChannel::Connected()
{
    return _connected;
}

bool MqttChannel::Send(const string& topic, const string& message, int qos)
{
    unique_lock<timed_mutex> lck(_mutex, std::defer_lock);
    if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
    {
        if (_connected)
        {
            return mosquitto_publish(_mosq, NULL, topic.c_str(), static_cast<int>(message.size()), message.c_str(), qos, 0) == 0;
        }
        else
        {
            return false;
        }
    }
    else
    {
        LogPool::Error(LogEvent::Thread, "mqtt send lock timeout");
        return false;
    }

}

bool MqttChannel::Send(const string& topic, const unsigned char* message, unsigned int size, int qos)
{
    unique_lock<timed_mutex> lck(_mutex, std::defer_lock);
    if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
    {
        if (_connected)
        {
            return mosquitto_publish(_mosq, NULL, topic.c_str(), size, message, qos, 0) == 0;
        }
        else
        {
            return false;
        }
    }
    else
    {
        LogPool::Error(LogEvent::Thread, "mqtt send lock timeout");
        return false;
    }
  
}

void MqttChannel::StartCore()
{
    //libmosquitto 库初始化
    mosquitto_lib_init();
    //创建mosquitto客户端
    _mosq = mosquitto_new(NULL, true, this);
    if (_mosq==NULL)
    {
        LogPool::Error(LogEvent::Mqtt, "mosquitto_new");
        mosquitto_destroy(_mosq);
        mosquitto_lib_cleanup();
        return;
    }
    else
    {
        LogPool::Information(LogEvent::Mqtt, "init mqtt");
    }

    _connected = mosquitto_connect(_mosq, _ip.c_str(), _port, KeepAlive) == 0;
    if (_connected)
    {
        LogPool::Information(LogEvent::Mqtt, "mqtt connect");
    }
    while (!_cancelled)
    {
        unique_lock<timed_mutex> lck(_mutex, std::defer_lock);
        if (lck.try_lock_for(chrono::seconds(ThreadObject::LockTime)))
        {
            if (_connected)
            {
                int i = mosquitto_loop(_mosq, 0, 1);
                if (i == MOSQ_ERR_NO_CONN)
                {
                    _connected = false;
                    LogPool::Information(LogEvent::Mqtt, "mqtt disconnect");
                }
            }
            else
            {
                _connected = mosquitto_reconnect(_mosq) == 0;
                if (_connected)
                {
                    LogPool::Information(LogEvent::Mqtt, "mqtt reconnect");
                }
            }
            lck.unlock();
        }
        else
        {
            LogPool::Error(LogEvent::Thread, "mqtt receive lock timeout");
        }

        if (_connected)
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