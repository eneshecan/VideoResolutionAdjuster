#include "colorspace_converter.h"


colorspace_converter::colorspace_converter() :
    src_height_(0)
{}

colorspace_converter::~colorspace_converter()
{}


int colorspace_converter::init(AVPixelFormat source_colorspace, AVPixelFormat target_colorspace,
                                int src_width, int src_height, int dst_width, int dst_height)
{
    src_height_ = src_height;

    dst_frame_ = av_frame_alloc();
    if(!dst_frame_)
        return -1;

    int numBytes = avpicture_get_size(target_colorspace, dst_width,
                                dst_height);

    buffer_ = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    avpicture_fill((AVPicture *)dst_frame_, buffer_, target_colorspace,
                   dst_width, dst_height);

    dst_frame_->format = target_colorspace;
    dst_frame_->width = dst_width;
    dst_frame_->height = dst_height;

    context_ = sws_getContext(src_width, src_height,
                              source_colorspace,
                              dst_width,dst_height,
                              target_colorspace,
                              SWS_BICUBIC, nullptr, nullptr, nullptr);

    return 0;
}

AVFrame *colorspace_converter::convert(AVFrame *src_frame) {
    sws_scale(context_, (uint8_t const * const *)src_frame->data,
              src_frame->linesize, 0, src_height_,
              dst_frame_->data, dst_frame_->linesize);

    return dst_frame_;
}


