#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "DetectChannel.h"
#include "DecodeChannel.h"
#include "TrafficDetector.h"
#include "SocketMaid.h"

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class TrafficStartup :public ThreadObject
        , public IObserver<HttpReceivedEventArgs>
    {
    public:

        /**
        * @brief: ���캯��
        */
        TrafficStartup();

        /**
        * @brief: ��������
        */
        virtual ~TrafficStartup();

        /**
        * @brief: ��ʼ��ϵͳ
        */
        bool Init();

        /**
        * @brief: http��Ϣ�����¼�����
        * @param: e http��Ϣ�����¼�����
        */
        void Update(HttpReceivedEventArgs* e);

        //ͨ������
        static const int ChannelCount;

    protected:

        void StartCore();

        /**
        * @brief: ������ʵ�ֵ��߳���ѯ����
        */
        virtual void PollCore() {};

        /**
        * @brief: ��ʼ������߼��༯��
        * @return: ����༯��
        */
        virtual std::vector<TrafficDetector*> InitDetectors() = 0;

        /**
        * @brief: ��ʼ��ͨ������
        */
        virtual void InitChannels() = 0;

        /**
        * @brief: ��ȡͨ��json����
        * @param: host �����ַ
        * @param: channelIndex ͨ�����
        * @return: ͨ��json����
        */
        virtual std::string GetChannelJson(const std::string& host, int channelIndex)=0;

        /**
        * @brief: ����ͨ������
        * @param: e http��Ϣ�����¼�����
        */
        virtual void SetDevice(HttpReceivedEventArgs* e)=0;

        /**
        * @brief: ����ͨ��
        * @param: e http��Ϣ�����¼�����
        */
        virtual void SetChannel(HttpReceivedEventArgs* e)=0;

        /**
        * @brief: ɾ��ͨ��
        * @param: channelIndex ͨ�����
        * @return: ɾ���ɹ�����true�����򷵻�false
        */
        virtual bool DeleteChannel(int channelIndex)=0;

        /**
        * @brief: ���ý�����
        * @param: channelIndex ͨ�����
        * @param: inputUrl ��ƵԴ��ַ
        * @param: outputUrl ��Ƶ�����ַ
        */
        void SetDecode(int channelIndex, const std::string& inputUrl, const std::string& outputUrl);
        
        /**
        * @brief: ɾ��������
        * @param: channelIndex ͨ�����
        */
        void DeleteDecode(int channelIndex);

        /**
        * @brief: ���ͨ�������Ϣ
        * @param: channelIndex ͨ�����
        * @return: ���ɹ����ؿ��ַ��������򷵻ش���ԭ��
        */
        std::string CheckChannel(int channelIndex);

        /**
        * @brief: �ж�ͨ������Ƿ����
        * @param: channelIndex ͨ�����
        * @return: ����ture��ʾͨ����ſ��ã����򷵻�false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * @brief: ��ȡ������Ϣjson����
        * @param: field �ֶ���
        * @param: message ������Ϣ
        * @return: json����
        */
        std::string GetErrorJson(const std::string& field, const std::string& message);

        //mqtt
        MqttChannel* _mqtt;

    private:
        /**
        * @brief: ��ѯ�豸
        * @param: e http��Ϣ�����¼�����
        */
        void GetDevice(HttpReceivedEventArgs* e);

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
        
        //ϵͳ����ʱ��
        DateTime _startTime;
        //�㷨�Ƿ��ʼ���ɹ�
        bool _sdkInited;
        //�㷨�汾
        std::string _sdkVersion;

        //socket����
        SocketMaid* _socketMaid;
        //http��Ϣ����
        HttpHandler _handler;
        //����̼߳��ϣ�������Ƶ����
        std::vector<DetectChannel*> _detects;
        //ʶ���̼߳��ϣ�������Ƶ����/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;
        //�����߳�ͬ����
        std::mutex _decodeMutex;
        //�����̼߳��ϣ�������Ƶ�����������Ƶ�����ΪNULL
        std::vector<DecodeChannel*> _decodes;

    };
}
