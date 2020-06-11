#include "EncodeHandler.h"

using namespace std;
using namespace OnePunchMan;

const int EncodeHandler::FPS = 25;

EncodeHandler::EncodeHandler(string* json, const string& filePath, int width, int height, int seconds)
	:_json(json),_filePath(filePath),_width(width),_height(height), _encodeContext(NULL), _outputContext(NULL)
	, _yuvFrame(NULL), _yuvSize(0), _pts(0)
{

	_count = seconds * FPS;

	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		LogPool::Error(LogEvent::Encode, "avcodec_find_encoder");
		return;
	}

	_encodeContext = avcodec_alloc_context3(codec);
	if (!_encodeContext)
	{
		LogPool::Error(LogEvent::Encode, "avcodec_alloc_context3");
		Uninit();
		return;
	}

	_encodeContext->width = width; // ���ñ�����Ƶ��� 
	_encodeContext->height = height; // ���ñ�����Ƶ�߶�
	_encodeContext->bit_rate = 1000; // ������(����)��Խ����Ƶ����Խ��
	_encodeContext->time_base.num = 1;
	_encodeContext->time_base.den = FPS; // ����֡�ʣ�numΪ���ӣ�denΪ��ĸ�������1/25���ʾ25֡/s
	_encodeContext->pix_fmt = AV_PIX_FMT_NV21; // ����������ظ�ʽ
	_encodeContext->codec_id = AV_CODEC_ID_H264; // ���ñ����ʽ
	_encodeContext->profile = FF_PROFILE_H264_BASELINE;
	_encodeContext->level = 40;
	_encodeContext->thread_count = 2; // �߳�����

	int result = avcodec_open2(_encodeContext, codec, NULL);
	if (result < 0)
	{
		LogPool::Error(LogEvent::Encode, "avcodec_open2");
		Uninit();
		return;
	}

	avformat_alloc_output_context2(&_outputContext, 0, 0, filePath.c_str());
	AVStream* stream= avformat_new_stream(_outputContext, NULL);
	stream->id = 0;
	stream->codecpar->codec_tag = 0;
	avcodec_parameters_from_context(stream->codecpar, _encodeContext);

	// ��������ռ�
	_yuvFrame = av_frame_alloc();
	_yuvFrame->width = width;
	_yuvFrame->height = height;
	_yuvFrame->format = AV_PIX_FMT_NV21;
	_yuvSize = static_cast<int>(width * height * 1.5);

	result = avio_open(&_outputContext->pb, filePath.c_str(), AVIO_FLAG_WRITE);
	if (result < 0)
	{
		LogPool::Error(LogEvent::Encode, "avio_open");
		Uninit();
		return;
	}
	// д��Ƶ�ļ�ͷ
	result = avformat_write_header(_outputContext, NULL);
	if (result < 0)
	{
		LogPool::Error(LogEvent::Encode, "avformat_write_header");
		Uninit();
		return;
	}
}

EncodeHandler::~EncodeHandler()
{
	Uninit();
}

void EncodeHandler::Uninit()
{
	if (_outputContext != NULL)
	{
		av_write_trailer(_outputContext);
		if (_outputContext->pb != NULL)
		{
			avio_close(_outputContext->pb);
		}
		avformat_free_context(_outputContext);
		_outputContext = NULL;
	}
	if (_encodeContext != NULL)
	{
		avcodec_close(_encodeContext);
		avcodec_free_context(&_encodeContext);
		_encodeContext = NULL;
	}

	if (_yuvFrame != NULL)
	{
		av_frame_free(&_yuvFrame);
		_yuvFrame = NULL;
	}
}

bool EncodeHandler::Finished()
{
	return _count <= 0;
}

void EncodeHandler::AddYuv(const unsigned char* yuvBuffer)
{
	if (_count<= 0|| _outputContext==NULL)
	{
		return;
	}	
	av_image_fill_arrays(_yuvFrame->data, _yuvFrame->linesize, yuvBuffer, AV_PIX_FMT_NV12, _width, _height, 1);
	for (int i = 0; i < 10; ++i)
	{
		_yuvFrame->pts = _pts;
		_pts = _pts + 3600;
		if (avcodec_send_frame(_encodeContext, _yuvFrame) == 0)
		{
			AVPacket pkt;
			av_init_packet(&pkt);
			if (avcodec_receive_packet(_encodeContext, &pkt) == 0)
			{
				av_interleaved_write_frame(_outputContext, &pkt);
			}
			else
			{
				av_packet_unref(&pkt);
			}
		}
		_count -= 1;
	}

	if (_count <= 0)
	{
		Uninit();
		if (_json != NULL)
		{
			FILE* file = fopen(_filePath.c_str(), "rb");
			unsigned char buffer[1024];
			string base64("data:video/mp4;base64,");
			if (file != NULL)
			{
				size_t size = 0;
				while ((size = fread(buffer, 1, 1024, file)) != 0)
				{
					StringEx::ToBase64String(buffer, static_cast<unsigned int>(size), &base64);
				}
				fclose(file);
			}
			JsonSerialization::SerializeValue(_json, "video", base64);
		}
		remove(_filePath.c_str());
	}
}