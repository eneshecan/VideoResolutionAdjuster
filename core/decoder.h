#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <iostream>
#include <string>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

struct metadata
{
    int bit_rate_;
    int video_stream_id_;
    int width_;
    int height_;
    struct frame_rate
    {
        int num_;
        int den_;
    } frame_rate_;

    int audio_stream_id;

    metadata() :
            video_stream_id_(0),
            width_(0),
            height_(0),
            frame_rate_({}),
            audio_stream_id(0)
    {}
};

class decoder {

public:
    decoder();
    ~decoder();

    int init(const char *filename, metadata &m_data);
    int get_next_frame(int &stream_id, AVFrame **frame, int &linesize);

private:
    AVFormatContext   *format_ctx_;
    AVCodecContext    *codec_ctx_orig_;
    AVCodecContext    *codec_ctx_;
    AVCodec           *codec_;
    AVFrame           *frame_;
    AVPacket          packet_;
    SwsContext        *sws_ctx_;

    metadata          metadata_;

};
