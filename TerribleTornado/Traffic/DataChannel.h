#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "LaneDetector.h"
#include "FlowChannelData.h"
#include "HttpHandler.h"

namespace TerribleTornado
{
    //���ݷַ����ռ��߳�
    class DataChannel :public Saitama::ThreadObject
        , public Saitama::IObserver<MqttReceivedEventArgs>
        , public Saitama::IObserver<Saitama::HttpReceivedEventArgs>
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
        void Update(Saitama::HttpReceivedEventArgs* e);

    protected:

        void StartCore();

    private:


        void UpdateChannel(const FlowChannel& channel);


        /**
        * @brief: ��json�����л�ȡ������
        * @param: json json����
        * @param: key ������ֶεļ�
        * @return: ������
        */
        void DeserializeDetectItems(std::map<std::string,DetectItem>* items, const Saitama::JsonDeserialization& jd, const std::string& key, long long timeStamp);

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
        void CollectFlow(const Saitama::DateTime& now);

        /**
        * @brief: ��ȡurl�Ƿ���ָ����ǰ׺
        * @param: url url
        * @param: key ǰ׺
        * @return: ����true��ʾurl��ָ��ǰ׺
        */
        bool UrlStartWith(const std::string& url, const std::string& key);

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

        //ͬ����
        std::mutex _channelMutex;
        //��Ƶͨ���̼߳���
        std::vector<std::vector<LaneDetector*>> _channels;

        //��һ���ռ����ݵķ���
        int _lastMinute;
    };
}

