#pragma once
#include "mosquitto.h"

#include "Observable.h"
#include "LogPool.h"
#include "Thread.h"

namespace Saitama
{
    //mqtt������Ϣ�¼�����
    class MqttReceivedEventArgs
    {
    public:

        /**
* @brief: ��ҳ��ȡ��־
* @param: logDIrectory ��־Ŀ¼
* @param: logName ��־����
* @param: logDate ��־����
* @param: logLevel ��־����Ϊ0ʱ��ѯ����
* @param: logEvent ��־�¼���Ϊ0ʱ��ѯ����
* @param: pageNum ��ҳҳ��
* @param: pageSize ��ҳ����
* @param: hasTotal �Ƿ��ѯ����
* @return: ��һ��������ʾ��־��ѯ������ϣ��ڶ���������ʾ���������hasTotalΪfalse�򷵻�0
*/
        MqttReceivedEventArgs()
            :Message()
        {

        }

        /**
        * @brief: ���캯��
        * @param: message mqtt���յ�����Ϣ
        */
        MqttReceivedEventArgs(const char* message)
            :Message(message)
        {

        }

        //mqtt���յ�����Ϣ
        std::string Message;
    };

    //mqtt�ͻ����߳�
    class MqttChannel:public ThreadObject
    {
    public:

        /**
        * @brief: ���캯��
        * @param: ip mqtt����˵�ַ
        * @param: port mqtt����˶˿ڣ�Ĭ��Ϊ1883
        */
        MqttChannel(const std::string& ip, int port=1883);

        /**
        * @brief: mqtt���ӻص�����
        * @param: mosq mosqʵ��
        * @param: userdata �Զ������ݣ���ʱ��ʾthisָ��
        * @param: result ���ӽ��
        */
        static void ConnectedEventHandler(struct mosquitto* mosq, void* userdata, int result);

        /**
        * @brief: mqtt������Ϣ�ص�����
        * @param: mosq mosqʵ��
        * @param: userdata �Զ������ݣ���ʱ��ʾthisָ��
        * @param: message ���յ�����Ϣ
        */
        static void ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);

        //mqtt������Ϣ�¼�
        Observable<MqttReceivedEventArgs> MqttReceived;
    protected:

        void StartCore();

    private:

        //���mqtt����������ʱ����(��)
        static const int KeepAlive;
        //����mqtt����˵�ʱ����(����)
        static const int ConnectSpan;

        //mqtt����˵�ַ
        std::string _ip;
        //mqtt����˶˿�
        int _port;


    };
}


