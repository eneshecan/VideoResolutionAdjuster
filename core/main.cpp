extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <iostream>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[32];
    int  y;

    // Open file
    sprintf(szFilename, "frames/frame%d.ppm", iFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile== nullptr)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, (size_t)width*3, pFile);

    // Close file
    fclose(pFile);
}

int main(int argc, char *argv[]) {
    AVFormatContext   *format_ctx = nullptr;
    uint              i;
    int               video_stream;
    AVCodecContext    *codec_ctx_orig = nullptr;
    AVCodecContext    *codec_ctx = nullptr;
    AVCodec           *codec = nullptr;
    AVFrame           *frame = nullptr;
    AVFrame           *frame_rgb = nullptr;
    AVPacket          packet = {};
    int               frame_finished;
    int               num_bytes;
    uint8_t           *buffer = nullptr;
    SwsContext *sws_ctx = nullptr;

    if(argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }
    // Register all formats and codecs
    av_register_all();

    // Open video file
    int result = avformat_open_input(&format_ctx, argv[1], nullptr, nullptr);
    if(result != 0)
        return -1; // Couldn't open file

    // Retrieve stream information
    if(avformat_find_stream_info(format_ctx, nullptr)<0) {
        return -1;
    } // Couldn't find stream information


    // Dump information about file onto standard error
    av_dump_format(format_ctx, 0, argv[1], 0);

    // Find the first video stream
    video_stream=-1;
    for(i=0; i<format_ctx->nb_streams; i++)
        if(format_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            video_stream=i;
            break;
        }
    if(video_stream==-1)
        return -1; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    codec_ctx_orig=format_ctx->streams[video_stream]->codec;
    // Find the decoder for the video stream
    codec=avcodec_find_decoder(codec_ctx_orig->codec_id);
    if(codec==nullptr) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Copy context
    codec_ctx = avcodec_alloc_context3(codec);
    if(avcodec_copy_context(codec_ctx, codec_ctx_orig) != 0) {
        fprintf(stderr, "Couldn't copy codec context");
        return -1; // Error copying codec context
    }

    // Open codec
    if(avcodec_open2(codec_ctx, codec, nullptr)<0)
        return -1; // Could not open codec

    // Allocate video frame
    frame=av_frame_alloc();

    // Allocate an AVFrame structure
    frame_rgb=av_frame_alloc();
    if(!frame_rgb)
        return -1;

    // Determine required buffer size and allocate buffer
    num_bytes=avpicture_get_size(PIX_FMT_RGB24, codec_ctx->width,
                                codec_ctx->height);
    buffer=(uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in frame_rgb
    // Note that frame_rgb is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill(reinterpret_cast<AVPicture*>(frame_rgb), buffer, PIX_FMT_RGB24,
                   codec_ctx->width, codec_ctx->height);

    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(codec_ctx->width,
                             codec_ctx->height,
                             codec_ctx->pix_fmt,
                             codec_ctx->width,
                             codec_ctx->height,
                             PIX_FMT_RGB24,
                             SWS_BILINEAR,
                             nullptr,
                             nullptr,
                             nullptr
    );

    // Read frames and save first five frames to disk
    i=0;
    while(av_read_frame(format_ctx, &packet)>=0) {
        // Is this a packet from the video stream?
        if(packet.stream_index==video_stream) {
            // Decode video frame
            avcodec_decode_video2(codec_ctx, frame, &frame_finished, &packet);

            // Did we get a video frame?
            if(frame_finished) {
                // Convert the image from its native format to RGB
                sws_scale(sws_ctx, (uint8_t const * const *)frame->data,
                          frame->linesize, 0, codec_ctx->height,
                          frame_rgb->data, frame_rgb->linesize);

                // Save the frame to disk
                i++;
                if(i>50 && i<61)
                    SaveFrame(frame_rgb, codec_ctx->width, codec_ctx->height, i);
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    // Free the RGB image
    av_free(buffer);
    av_frame_free(&frame_rgb);

    // Free the YUV frame
    av_frame_free(&frame);

    // Close the codecs
    avcodec_close(codec_ctx);
    avcodec_close(codec_ctx_orig);

    // Close the video file
    avformat_close_input(&format_ctx);

    return 0;
}
