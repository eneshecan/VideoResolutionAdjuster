#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <iostream>
#include <string>
#include "decoder.h"

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

class encoder {
public:
    encoder();
    ~encoder();

    int init(const char *filename, metadata m_data);
    int encode_frame(AVFrame *frame, int frame_count);
    void finalize();

private:
    AVCodec *codec_;
    AVCodecContext *codec_ctx_;
    AVFrame *frame_;
    uint8_t *outbuf_;
    uint8_t *frame_buf_;
    FILE *f;
    size_t outbuf_size_;
    size_t frame_size_;
};