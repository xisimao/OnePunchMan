//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
////#include "mosquitto.h"
////#include "Sqlite.h"
//
//using namespace std;
//using namespace Saitama;
//
//#define HOST "localhost"
//#define PORT  1884
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
//
//    string sql("Select * FROM Flow_Channel");
//    string error;
//    Sqlite sqlite;
//    sqlite3_stmt* stmt = sqlite.ExecuteRows(sql, &error);
//    if (stmt != NULL)
//    {
//        while (sqlite3_step(stmt) == SQLITE_ROW) {
//
//            string channelId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) == NULL ?
//                "" : string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
//            string channelName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) == NULL ?
//                "" : string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
//            int channelIndex = sqlite3_column_int(stmt, 2);
//            int channelType = sqlite3_column_int(stmt, 3);
//            string rtspUser = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) == NULL ?
//                "" : string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
//            string rtspPwd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) == NULL ?
//                "" : string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
//            int rtspProtocol = sqlite3_column_int(stmt, 6);
//            bool isLoop = sqlite3_column_int(stmt, 7);
//            cout << channelId << endl;
//            cout << channelName << endl;
//        }
//        sqlite3_finalize(stmt);
//    }
//
//
//    struct mosquitto* mosq = NULL;
//    //libmosquitto 库初始化
//    mosquitto_lib_init();
//    //创建mosquitto客户端
//    mosq = mosquitto_new(NULL, session, NULL);
//    if (!mosq) {
//        printf("create client failed..\n");
//        mosquitto_lib_cleanup();
//        return 1;
//    }
//    //设置回调函数，需要时可使用
//    mosquitto_log_callback_set(mosq, my_log_callback);
//    mosquitto_connect_callback_set(mosq, my_connect_callback);
//    mosquitto_message_callback_set(mosq, my_message_callback);
//    mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
//    //客户端连接服务器
//    if (mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)) {
//        fprintf(stderr, "Unable to connect.\n");
//        return 1;
//    }
//    //循环处理网络消息
//    mosquitto_loop_forever(mosq, -1, 1);
//
//    mosquitto_destroy(mosq);
//    mosquitto_lib_cleanup();
//
//    return 0;
//}