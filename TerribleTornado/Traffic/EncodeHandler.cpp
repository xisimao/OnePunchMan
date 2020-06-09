#include "EncodeHandler.h"

using namespace std;
using namespace OnePunchMan;

const int EncodeHandler::FPS = 25;

EncodeHandler::EncodeHandler(string* json, const string& filePath, int width, int height, int seconds)
	:_json(json),_filePath(filePath),_width(width),_height(height), _encodeContext(NULL), _outputContext(NULL), _swsContext(NULL)
	, _yuv420pFrame(NULL)
	, _yuvSize(0), _pts(0)
{

	_count = seconds * FPS;

	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
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
	_encodeContext->bit_rate = 1000; // ������(����)��Խ����Ƶ����Խ��
	_encodeContext->width = width; // ���ñ�����Ƶ��� 
	_encodeContext->height = height; // ���ñ�����Ƶ�߶�
	_encodeContext->time_base.num = 1;
	_encodeContext->time_base.den = FPS; // ����֡�ʣ�numΪ���ӣ�denΪ��ĸ�������1/25���ʾ25֡/s

	_encodeContext->gop_size = 50; // �������С���ؼ�֡
	_encodeContext->max_b_frames = 0; // ����B֡�����,��ֵ��ʾ��������B֮֡�䣬����������B֡�����֡��

	_encodeContext->pix_fmt = AV_PIX_FMT_YUV420P; // ����������ظ�ʽ
	_encodeContext->codec_id = AV_CODEC_ID_MPEG4; // ���ñ����ʽ
	_encodeContext->thread_count = 8; // �߳�����
	_encodeContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; // ȫ�ֵı�����Ϣ

	int result = avcodec_open2(_encodeContext, codec, NULL);
	if (result < 0)
	{
		LogPool::Error(LogEvent::Encode, "avcodec_open2");
		Uninit();
		return;
	}

	avformat_alloc_output_context2(&_outputContext, 0, 0, filePath.c_str());
	AVStream* st = avformat_new_stream(_outputContext, NULL);
	st->id = 0;
	st->codecpar->codec_tag = 0;
	avcodec_parameters_from_context(st->codecpar, _encodeContext);

	_swsContext = sws_getContext(width, height, AV_PIX_FMT_NV12, width, height, AV_PIX_FMT_YUV420P,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);

	// ��������ռ�
	_yuv420pFrame = av_frame_alloc();
	_yuv420pFrame->width = width;
	_yuv420pFrame->height = height;
	_yuv420pFrame->linesize[0] = width;
	_yuv420pFrame->linesize[1] = width/2;
	_yuv420pFrame->linesize[2] = width/2;
	_yuv420pFrame->format = AV_PIX_FMT_YUV420P;

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
	if (_swsContext != NULL)
	{
		sws_freeContext(_swsContext);
		_swsContext = NULL;
	}

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

	if (_yuv420pFrame != NULL)
	{
		av_frame_free(&_yuv420pFrame);
		_yuv420pFrame = NULL;
	}
}

bool EncodeHandler::Finished()
{
	return _count <= 0;
}

void EncodeHandler::AddYuv(unsigned char* yuv420pBuffer)
{
	if (_count<= 0|| _outputContext==NULL)
	{
		return;
	}
	_yuv420pFrame->data[0] = yuv420pBuffer;
	_yuv420pFrame->data[1] = _yuv420pFrame->data[0] +_width*_height;
	_yuv420pFrame->data[2] = _yuv420pFrame->data[1] +_width*_height/4;
	_yuv420pFrame->pts = _pts;
	_pts = _pts + 3600;
	if (avcodec_send_frame(_encodeContext, _yuv420pFrame) == 0)
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
	if (_count == 0)
	{
		Uninit();
		FILE* file = fopen(_filePath.c_str(), "rb");
		unsigned char buffer[1024];
		string base64;
		if (file != NULL)
		{
			unsigned int size = 0;
			while ((size = fread(buffer, 1, 1024, file)) != 0)
			{
				StringEx::ToBase64String(buffer, size, &base64);
			}
			fclose(file);
		}
		remove(_filePath.c_str());
		JsonSerialization::SerializeValue(_json,"video", base64);
	}
}