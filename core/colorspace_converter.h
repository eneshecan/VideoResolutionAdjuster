#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class colorspace_converter {

public:
    colorspace_converter();
    ~colorspace_converter();

    int init(AVPixelFormat source_colorspace, AVPixelFormat target_colorspace, int src_width, int src_height, int dst_width, int dst_height);
    AVFrame *convert(AVFrame *src_frame);

private:
    SwsContext    *context_;
    AVFrame       *dst_frame_;
    int           src_height_;
    uint8_t       *buffer_;
};
