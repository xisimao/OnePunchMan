#include <cstdio>

int main()
{
    printf("hello from Genos!\n");
    return 0;
}



//#include <stdio.h>
//#include <stdlib.h>
//#include <mosquitto.h>
//#include <string.h>
//
//#define HOST "localhost"
//#define PORT  1883
//#define KEEP_ALIVE 60
//
//bool session = true;
//
//void my_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
//{
//    if (message->payloadlen) {
//        printf("%s %s", message->topic, message->payload);
//    }
//    else {
//        printf("%s (null)\n", message->topic);
//    }
//    fflush(stdout);
//}
//int iii = 1;
//void my_connect_callback(struct mosquitto* mosq, void* userdata, int result)
//{
//
//    if (!result) {
//        /* Subscribe to broker information topics on successful connect. */
//        mosquitto_subscribe(mosq, &iii, "test", 2);
//    }
//    else {
//        fprintf(stderr, "Connect failed\n");
//    }
//}
//
//void my_subscribe_callback(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos)
//{
//    int i;
//    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
//    for (i = 1; i < qos_count; i++) {
//        printf(", %d", granted_qos[i]);
//    }
//    printf("\n");
//}
//
//void my_log_callback(struct mosquitto* mosq, void* userdata, int level, const char* str)
//{
//    /* Pring all log messages regardless of level. */
//    printf("log:%s\n", str);
//}
//
//int main()
//{
//    struct mosquitto* mosq = NULL;
//    //libmosquitto ���ʼ��
//    mosquitto_lib_init();
//    //����mosquitto�ͻ���
//    mosq = mosquitto_new(NULL, session, NULL);
//    if (!mosq) {
//        printf("create client failed..\n");
//        mosquitto_lib_cleanup();
//        return 1;
//    }
//    //���ûص���������Ҫʱ��ʹ��
//    mosquitto_log_callback_set(mosq, my_log_callback);
//    mosquitto_connect_callback_set(mosq, my_connect_callback);
//    mosquitto_message_callback_set(mosq, my_message_callback);
//    mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
//    //�ͻ������ӷ�����
//    if (mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)) {
//        fprintf(stderr, "Unable to connect.\n");
//        return 1;
//    }
//    //ѭ������������Ϣ
//    mosquitto_loop_forever(mosq, -1, 1);
//
//    mosquitto_destroy(mosq);
//    mosquitto_lib_cleanup();
//
//    return 0;
//}