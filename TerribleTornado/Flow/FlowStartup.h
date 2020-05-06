#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "SeemmoSDK.h"
#include "FlowData.h"
#include "MqttChannel.h"
#include "DetectChannel.h"
#include "DecodeChannel.h"
#include "FlowChannelDetector.h"
#include "SocketMaid.h"

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class FlowStartup :public ThreadObject
        , public IObserver<HttpReceivedEventArgs>
    {
    public:

        /**
        * @brief: ���캯��
        */
        FlowStartup();

        /**
        * @brief: ��������
        */
        ~FlowStartup();

        /**
        * @brief: ��ʼ��ϵͳ
        */
        bool Init();

        /**
        * @brief: http��Ϣ�����¼�����
        * @param: e http��Ϣ�����¼�����
        */
        void Update(HttpReceivedEventArgs* e);

    protected:

        void StartCore();

    private:

        /**
        * @brief: ��ѯ�豸
        * @param: e http��Ϣ�����¼�����
        */
        void GetDevice(HttpReceivedEventArgs* e);
        
        /**
        * @brief: ����ͨ������
        * @param: e http��Ϣ�����¼�����
        */
        void SetDevice(HttpReceivedEventArgs* e);
        
        /**
        * @brief: ��ȡͨ��
        * @param: e http��Ϣ�����¼�����
        */
        void GetChannel(HttpReceivedEventArgs* e);
  
        /**
        * @brief: ����ͨ��
        * @param: e http��Ϣ�����¼�����
        */
        void SetChannel(HttpReceivedEventArgs* e);
        
        /**
        * @brief: ɾ��ͨ��
        * @param: e http��Ϣ�����¼�����
        */
        void DeleteChannel(HttpReceivedEventArgs* e);
        
        /**
        * @brief: ����ͨ��
        * @param: channel ͨ��
        */
        void SetChannel(const FlowChannel& channel);
        
        /**
        * @brief: ɾ��ͨ��
        * @param: channelIndex ͨ�����
        */
        void DeleteChannel(int channelIndex);

        /**
        * @brief: �ж�ͨ������Ƿ����
        * @param: channelIndex ͨ�����
        * @return: ����ture��ʾͨ����ſ��ã����򷵻�false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * @brief: ���ͨ����Ϣ
        * @param: channel ͨ��
        * @return: ���ɹ����ؿ��ַ��������򷵻ش���ԭ��
        */
        std::string CheckChannel(const FlowChannel& channel);

        /**
        * @brief: ��ȡͨ��json����
        * @param: e http��Ϣ�����¼�����
        * @param: channel ͨ��
        * @return: ͨ��json����
        */
        std::string GetChannelJson(HttpReceivedEventArgs* e,const FlowChannel& channel);

        /**
        * @brief: ��ȡurl�Ƿ���ָ����ǰ׺
        * @param: url url
        * @param: key ǰ׺
        * @return: ����true��ʾurl��ָ��ǰ׺
        */
        bool UrlStartWith(const std::string& url, const std::string& key);
       
        /**
        * @brief: ��ȡurl�е�id
        * @param: url url
        * @param: key ǰ׺
        * @return: id
        */
        std::string GetId(const std::string& url, const std::string& key);
       
        /**
        * @brief: ��ȡ������Ϣjson����
        * @param: field �ֶ���
        * @param: message ������Ϣ
        * @return: json����
        */
        std::string GetErrorJson(const std::string& field, const std::string& message);

        //����mqtt����
        static const std::string FlowTopic;

        //�㷨�Ƿ��ʼ���ɹ�
        bool _sdkInited;

        //socket����
        SocketMaid* _socketMaid;
        //http��Ϣ����
        HttpHandler _handler;
        //mqtt
        MqttChannel* _mqtt;
        //����̼߳��ϣ�������Ƶ����
        std::vector<DetectChannel*> _detects;
        //ʶ���̼߳��ϣ�������Ƶ����/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;
        //ͨ����⼯�ϣ�������Ƶ����
        std::vector<FlowChannelDetector*> _detectors;
        //�����߳�ͬ����
        std::mutex _decodeMutex;
        //�����̼߳��ϣ�������Ƶ�����������Ƶ�����ΪNULL
        std::vector<DecodeChannel*> _decodes;

        //ϵͳ����ʱ��
        DateTime _startTime;
        //��һ���ռ����ݵķ���
        int _lastMinute;
    };
}

