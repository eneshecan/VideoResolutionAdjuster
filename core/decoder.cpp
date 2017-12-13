#include "decoder.h"

decoder::decoder() :
        format_ctx_(nullptr),
        codec_ctx_orig_(nullptr),
        codec_ctx_(nullptr),
        frame_(nullptr),
        frame_rgb_(nullptr),
        packet_({}),
        buffer_(nullptr),
        sws_ctx_(nullptr)
{

}

decoder::~decoder() {
    av_free(buffer_);
    av_frame_free(&frame_rgb_);

    av_frame_free(&frame_);

    avcodec_close(codec_ctx_);
    avcodec_close(codec_ctx_orig_);

    avformat_close_input(&format_ctx_);
}

/* Returns -1 in case of failure */
int decoder::init(const char *filename, metadata &m_data) {
    av_register_all();

    int result = avformat_open_input(&format_ctx_, filename, nullptr, nullptr);

    if(result != 0)
        return -1;

    if(avformat_find_stream_info(format_ctx_, nullptr)<0) {
        return -1;
    }

    av_dump_format(format_ctx_, 0, filename, 0);

    for(uint i=0; i<format_ctx_->nb_streams; i++)
        if(format_ctx_->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            metadata_.video_stream_id_ = i;
        }
        else if(format_ctx_->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            metadata_.audio_stream_id = i;
            break;
        }

    if(metadata_.video_stream_id_ == -1) {
        return -1;
    }

    codec_ctx_orig_=format_ctx_->streams[metadata_.video_stream_id_]->codec;

    codec_= avcodec_find_decoder(codec_ctx_orig_->codec_id);
    if(!codec_) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    codec_ctx_ = avcodec_alloc_context3(codec_);
    if(avcodec_copy_context(codec_ctx_, codec_ctx_orig_) != 0) {
        fprintf(stderr, "Couldn't copy codec context");
        return -1;
    }

    if(avcodec_open2(codec_ctx_, codec_, nullptr) < 0)
        return -1;

    metadata_.width_ = codec_ctx_->width;
    metadata_.height_ = codec_ctx_->height;
    metadata_.frame_rate_.num_ = codec_ctx_->framerate.num;
    metadata_.frame_rate_.den_ = codec_ctx_->framerate.den;

    frame_=av_frame_alloc();
    if(!frame_)
        return -1;

    frame_rgb_=av_frame_alloc();
    if(!frame_rgb_)
        return -1;

    int num_bytes = avpicture_get_size(PIX_FMT_RGB24, codec_ctx_->width,
                                 codec_ctx_->height);

    buffer_=(uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));

    avpicture_fill(reinterpret_cast<AVPicture*>(frame_rgb_), buffer_, PIX_FMT_RGB24,
                   codec_ctx_->width, codec_ctx_->height);

    sws_ctx_ = sws_getContext(codec_ctx_->width,
                             codec_ctx_->height,
                             codec_ctx_->pix_fmt,
                             codec_ctx_->width,
                             codec_ctx_->height,
                             PIX_FMT_RGB24,
                             SWS_BILINEAR,
                             nullptr,
                             nullptr,
                             nullptr
    );

    m_data = metadata_;

    return 0;
}

int decoder::get_next_frame(int &stream_id, AVFrame **frame_rgb, int &linesize) {

    if (av_read_frame(format_ctx_, &packet_) < 0)
        return -1;

    if(packet_.stream_index == metadata_.video_stream_id_) {
        stream_id = metadata_.video_stream_id_;

        int frame_finished = 0;

        int res = avcodec_decode_video2(codec_ctx_, frame_, &frame_finished, &packet_);

        if(frame_finished) {

            sws_scale(sws_ctx_, (uint8_t const * const *)frame_->data,
                      frame_->linesize, 0, codec_ctx_->height,
                      frame_rgb_->data, frame_rgb_->linesize);

            *frame_rgb = frame_rgb_;
        }
    }

    av_free_packet(&packet_);

    return 0;
}
