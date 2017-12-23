#include "encoder.h"

encoder::encoder() :
    codec_(nullptr),
    codec_ctx_(nullptr),
    frame_(nullptr),
    outbuf_(nullptr),
    frame_buf_(nullptr),
    outbuf_size_(0),
    frame_size_(0)
{}


encoder::~encoder() {
    if(f)
        fclose(f);

    free(frame_buf_);
    free(outbuf_);

    avcodec_close(codec_ctx_);
    av_free(codec_ctx_);
    av_free(frame_);
}

int encoder::init(std::string filename, std::string resolution, metadata m_data) {
    avcodec_register_all();

    /* find the mpeg1 video encoder */
    codec_ = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!codec_) {
        fprintf(stderr, "Codec not found!\n");
        return -1;
    }

    codec_ctx_ = avcodec_alloc_context3(codec_);
    frame_ = av_frame_alloc();

    /* put sample parameters */
    codec_ctx_->bit_rate = m_data.bit_rate_;
    /* resolution must be a multiple of two */
    codec_ctx_->width = m_data.width_;
    codec_ctx_->height = m_data.height_;
    /* frames per second */
    codec_ctx_->time_base= AVRational{m_data.frame_rate_.den_, m_data.frame_rate_.num_};
    codec_ctx_->gop_size = 5; /* emit one intra frame every five frames */
    codec_ctx_->max_b_frames = 1;
    codec_ctx_->pix_fmt = PIX_FMT_YUV420P;

    /* open it */
    if (avcodec_open2(codec_ctx_, codec_, nullptr) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    std::string plain_filename = filename.substr(0, filename.find('.'));
    std::string target_filename = plain_filename + "-" + resolution + ".mp4";

    f = fopen(target_filename.c_str(), "wb");
    if (!f) {
        fprintf(stderr, "could not open %s\n", target_filename);
        return -1;
    }

    /* alloc image and output buffer */
    outbuf_size_ = 1500000; // Check if 8K fits into this
    outbuf_ = reinterpret_cast<uint8_t*>(malloc(outbuf_size_));
    frame_size_ = static_cast<size_t>(codec_ctx_->width * codec_ctx_->height);
    frame_buf_ = reinterpret_cast<uint8_t*>(malloc(frame_size_ * 3)); /* size for YUV420 */

    frame_->data[0] = frame_buf_;
    frame_->data[1] = frame_->data[0] + frame_size_;
    frame_->data[2] = frame_->data[1] + frame_size_ / 4;
    frame_->linesize[0] = codec_ctx_->width;
    frame_->linesize[1] = codec_ctx_->width / 2;
    frame_->linesize[2] = codec_ctx_->width / 2;

    return 0;
}

int encoder::encode_frame(AVFrame *frame, int frame_count) {
    int out_size = avcodec_encode_video(codec_ctx_, outbuf_, static_cast<int>(outbuf_size_), frame);

    if(out_size < 0)
        return -1;

    printf("encoding frame %3d (size=%5d)\n", frame_count, out_size);
    fwrite(outbuf_, 1, static_cast<size_t>(out_size), f);

    return 0;
}

void encoder::finalize() {
    /* add sequence end code to have a real mpeg file */
    outbuf_[0] = 0x00;
    outbuf_[1] = 0x00;
    outbuf_[2] = 0x01;
    outbuf_[3] = 0xb7;
    fwrite(outbuf_, 1, 4, f);
}
