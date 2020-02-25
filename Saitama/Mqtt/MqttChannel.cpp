#include "MqttChannel.h"

using namespace std;
using namespace Saitama;

MqttChannel::MqttChannel(const string& ip, int port)
    :ThreadObject("mqtt"),_ip(ip),_port(port)
{

}

void MqttChannel::ConnectedEventHandler(struct mosquitto* mosq, void* userdata, int result)
{
    LogPool::Information(LogEvent::Mqtt, "mqtt connect success",result);
    mosquitto_subscribe(mosq, NULL, "aiservice/level_1_imageresult", 2);
}

void MqttChannel::ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
    MqttReceivedEventArgs args((const char*)message->payload);
    static_cast<MqttChannel*>(userdata)->MqttReceived.Notice(&args);
}

void MqttChannel::StartCore()
{
    struct mosquitto* mosq = NULL;
        //libmosquitto 库初始化
        mosquitto_lib_init();
        //创建mosquitto客户端
        mosq = mosquitto_new(NULL, true, this);
        if (mosq) 
        {
            LogPool::Information(LogEvent::Mqtt, "mqtt init success");
        }
        else
        {
            LogPool::Information(LogEvent::Mqtt, "mqtt init failed");
            mosquitto_lib_cleanup();
            return;
        }

        mosquitto_connect_callback_set(mosq, ConnectedEventHandler);
        mosquitto_message_callback_set(mosq, ReceivedEventHandler);

        int connectResult = -1;
        while (!Cancelled())
        {
            if (connectResult ==0)
            {
               int i= mosquitto_loop(mosq, -1, 1);
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
                connectResult = mosquitto_connect(mosq, _ip.c_str(), _port, 60);
            }
            if (connectResult != 0)
            {
                this_thread::sleep_for(chrono::milliseconds(5000));
            }
        }
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
}