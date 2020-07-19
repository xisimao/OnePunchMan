#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "HisiEncodeChannel.h"
#include "OutputHandler.h"
#include "DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    //DecodeChannel::InitFFmpeg();
    unsigned char* recognBgrBuffer = new unsigned char[1920 * 1080 * 3];
    unsigned char* recognJpgBuffer = tjAlloc(1920 * 1080 * 3);
    cv::Mat image(1080, 1920, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Rect rect(100,100,100,100);
    cv::Point point(200,200);
    cv::circle(image, point,10, cv::Scalar(0, 04, 255), -1);
    cv::rectangle(image, rect, cv::Scalar(0, 04, 255), 10);
    cv::putText(image, "abc", cv::Point(100, 100), cv::FONT_HERSHEY_COMPLEX, 3, cv::Scalar(0, 0, 255),3);
    //OnePunchMan::Rectangle rec(100,100,100,100);
    //ImageConvert::DrawRectangle(&image, rec, cv::Scalar(0, 0, 255));

    IVEHandler handler1;
    handler1.HandleFrame(image.data, 1920, 1080, 1);
    BGR24Handler handler;
    handler.HandleFrame(image.data, 1920, 1080, 1);
    int jpgSize = ImageConvert::BgrToJpg(image.data, 1920, 1080, recognJpgBuffer, 1920 * 1080 * 3);
    ImageConvert::JpgToFile(recognJpgBuffer, jpgSize, 1, 1);

    return 0;
}
