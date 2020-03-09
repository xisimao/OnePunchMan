#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "VideoDetector.h"
#include "FlowChannelData.h"
#include "HttpHandler.h"

namespace Saitama
{


    //���ݷַ����ռ��߳�
    class DataChannel :public ThreadObject
        , public IObserver<MqttReceivedEventArgs>
        , public IObserver<HttpReceivedEventArgs>
    {
    public:

        /**
        * @brief: ���캯��
        * @param: ip mqtt����˵�ַ
        * @param: port mqtt����˶˿ڣ�Ĭ��Ϊ1883
        */
        DataChannel(const std::string& ip, int port);

        /**
        * @brief: mqtt��Ϣ�����¼�����
        * @param: e mqtt��Ϣ�����¼�����
        */
        void Update(MqttReceivedEventArgs* e);

        /**
        * @brief: http��Ϣ�����¼�����
        * @param: e http��Ϣ�����¼�����
        */
        void Update(HttpReceivedEventArgs* e);

    protected:

        void StartCore();

    private:

        /**
        * @brief: ��json�����л�ȡ������
        * @param: json json����
        * @param: key ������ֶεļ�
        * @return: ������
        */
        std::vector<DetectItem> GetDetectItems(const std::string& json, const std::string& key, long long timeStamp);
        
        /**
        * @brief: ��json�����л�ȡ�����
        * @param: json json����
        * @return: �����
        */
        DetectItem GetDetectItem(const std::string& json);

        /**
        * @brief: ����������
        * @param: json json����
        */
        void HandleDetect(const std::string& json);

        /**
        * @brief: ����ʶ������
        * @param: json json����
        */
        void HandleRecognize(const std::string& json);

        /**
        * @brief: �ռ���������
        * @param: now ��ǰʱ��
        */
        void CollectFlow(const DateTime& now);

        /**
        * @brief: ��ȡurl�Ƿ���ָ����ǰ׺
        * @param: url url
        * @param: key ǰ׺
        * @return: ����true��ʾurl��ָ��ǰ׺
        */
        bool UrlStartWith(const std::string& url, const std::string& key);

        /**
        * @brief: ��ȡurl�еı��
        * @param: url url
        * @param: key ǰ׺
        * @return: ��ȡ�ɹ����ر�ŷ��򷵻ؿ��ַ���
        */
        std::string GetId(const std::string& url, const std::string& key);

        /**
        * @brief: ����ͨ����Ϣ
        * @param: channel ͨ����Ϣ
        */
        void UpdateChannel(const FlowChannel& channel);

        //������� sdk->����
        static const std::string DetectTopic;
        //ʶ������ sdk->����
        static const std::string RecognizeTopic;
        //���� ����->web
        static const std::string FlowTopic;
        //���� ����->web
        static const std::string VideoTopic;
        //IO״̬ ����->web
        static const std::string IOTopic;

        //mqtt�ͻ���
        MqttChannel* _mqtt;

        //mqtt����˵�ַ
        std::string _ip;
        //mqtt����˶˿�
        int _port;

        //��Ƶͨ���̼߳���
        std::vector<VideoDetector*> _channels;

        //��һ���ռ����ݵķ���
        int _lastMinute;
    };
}

