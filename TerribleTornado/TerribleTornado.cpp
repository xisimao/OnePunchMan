#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LogPool.h"
#include <string>
#include <iostream>

#include "FFmpegChannel.h"
#include "DataChannel.h"

using namespace std;
using namespace OnePunchMan;

int main1(int argc, char* argv[])
{
    if (argc > 1)
    {
        string arg(argv[1]);
        if (arg.compare("stop") == 0)
        {
            SocketMaid maid(2);
            SocketHandler handler;
            maid.AddConnectEndPoint(EndPoint("192.168.201.139", 7772), &handler);
            maid.Start();
            this_thread::sleep_for(chrono::seconds(5));
            stringstream ss;
            ss << "GET /api/system HTTP/1.1\r\n"
                << "\r\n"
                << "\r\n";
            maid.SendTcp(EndPoint("192.168.201.139", 7772), ss.str());
        }
    }
    else
    {
        DataChannel channel;
        channel.Start();
        channel.Join();
    }

    return 0;
}


int main()
{
    FFmpegChannel::InitFFmpeg();
    MqttChannel mqtt("192.168.201.19", 1883);
    mqtt.Start();
    FFmpegChannel ffmpeg(StringEx::Combine("ncdj.mp4"), "", true, &mqtt);
    ffmpeg.Start();
    system("pause");

    //int width = 2752;
    //int height = 2208;
    //FILE* file = fopen("bgr24_1.bmp", "rb");
    //uint8_t* bmp = new uint8_t[width * height *3+54];
    //fread(bmp, 1, width * height * 3 + 54, file);
    //fclose(file);
    //uint8_t* data = bmp + 54;


    //uint32_t step = width;
    //cv::Mat image(height, width, CV_8UC3,bmp+54);
   /* int src_b_index = 0;
    int src_g_index = src_b_index + height * step;
    int src_r_index = src_g_index + height * step;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            int dst_element_index = h * image.step + w * 3;
            image.data[dst_element_index] = data[dst_element_index];
            image.data[dst_element_index + 1] = data[dst_element_index+1];
            image.data[dst_element_index + 2] = data[dst_element_index+2];
        }
    }*/

    ////在原图画一个圆圈点
    //cv::Point point;//特征点，用以画在图像中  
    //point.x = 20;//特征点在图像中横坐标  
    //point.y = 50;//特征点在图像中纵坐标  
    //cv::circle(image, point, 4, cv::Scalar(0, 0, 255));//在图像中画出特征点，2是圆的半径 

    ////在原图画一条直线
    //cv::Point start = cv::Point(10, 100); //直线起点
    //cv::Point end = cv::Point(50, 200);   //直线终点
    //cv::line(image, start, end, cv::Scalar(0, 0, 255));

    ////在原图某个位置添加文字标记
    ////char str[4];
    ////int num = 100;
    ////_itoa_s(num, str, 10);//数字需要转为字符串来显示
    //string str = "Love100";
    //putText(image, str, end, cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

/*    vector<cv::Point> points;
    points.push_back(cv::Point(100, 100));
    points.push_back(cv::Point(200, 100));
    points.push_back(cv::Point(200, 200));
    points.push_back(cv::Point(100, 200));
    vector<cv::Point*> points1;
    points1.push_back(points.data());

    int npt[] = { 4 };
    cv::polylines(image, const_cast<const cv::Point**>(points1.data()), npt,1, true,cv::Scalar(0, 255, 255),10);*///在img图中画一条黄线，线宽为20，线型为8联通（一般都设置为8）
    //cv::line(image, cv::Point(10, 10), cv::Point(100, 250), cv::Scalar(0, 255, 255), 20, 8);//在img图中画一条黄线，线宽为20，线型为8联通（一般都设置为8）
    //cv::circle(image, cv::Point(200, 100), 50, cv::Scalar(0, 255, 255), 15, 8);   //以（200,100）为圆心，半径为50，线宽为15画空心圆
    //cv::circle(image, cv::Point(200, 250), 50, cv::Scalar(0, 255, 255), -1, 8);   //设置为-1时，画实心圆
    //cv::rectangle(image, cv::Point(10, 100), cv::Point(200, 200), cv::Scalar(0, 0, 255), 1, 8);   //传入坐上、右下角坐标，画空心矩形
    //cv::rectangle(image, cv::Rect(200, 300, 100, 50), cv::Scalar(0, 255, 0), -1, 8); //传矩形数据（左上角坐标（200,300）和宽100，高50），画实心绿色矩形

    //std::vector<int> param(2);
    //param[0] = cv::IMWRITE_JPEG_QUALITY;
    //param[1] = 10;
    //std::vector<unsigned char> buff;
    //cv::imencode(".jpg", image, buff, param);

    //FILE* fw = fopen("D:\\code\\OnePunchMan\\OnePunchMan\\output.jpg", "wb");
    //fwrite(buff.data(), 1, buff.size(), fw);
    //fclose(fw);
    return 0;
}