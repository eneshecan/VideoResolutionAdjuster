extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "decoder.h"
#include "encoder.h"
#include "colorspace_converter.h"

#include <fstream>
void resize(uint8_t* input, uint8_t* output, int sourceWidth, int sourceHeight, int targetWidth, int targetHeight) {
    int x, y, index;
    uint8_t *a;
    float x_ratio = ((float) (sourceWidth - 1)) / targetWidth;
    float y_ratio = ((float) (sourceHeight - 1)) / targetHeight;
    float blue, red, green;
    int offset = 0;

    for (int i = 0; i < targetHeight; i++) {
        for (int j = 0; j < targetWidth; j++) {
            x = (int) (x_ratio * j);
            y = (int) (y_ratio * i);

            index = (y * sourceWidth + x) * 3;
            a = &input[ index ];

            // blue element
            blue = *(a + 2);
            // green element
            green = *(a + 1);
            // red element
            red = *a;

            output[ offset++ ] = static_cast<uint8_t>(red);
            output[ offset++ ] = static_cast<uint8_t>(green);
            output[ offset++ ] = static_cast<uint8_t>(blue);


        }
    }
}

void save_frame_txt(AVFrame *frame_rgb, int width, int height, int frame_count)
{
    std::ofstream frame_file;
    frame_file.open("frames/frame"+std::to_string(frame_count)+".txt", std::ios::out);

    for(int i=0; i<width*height*3; ++i)
    {
        frame_file << std::to_string(frame_rgb->data[0][i]) << " ";
    }

    frame_file << std::endl;

    frame_file.close();
}

/* Saves RGB frame onto disk */
void save_frame(AVFrame *frame, int width, int height, int frame_count) {
    FILE *pFile;
    char szFilename[32];
    int  y;

    // Open file
    sprintf(szFilename, "frames/frame%d.ppm", frame_count);
    pFile=fopen(szFilename, "wb");
    if(pFile== nullptr)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(frame->data[0] + y* frame->linesize[0], 1, (size_t)width*3, pFile);

    fclose(pFile);
}

int init_frame(AVFrame **frame, int width, int height, AVPixelFormat pixelFormat)
{
    *frame = av_frame_alloc();
    if(*frame==nullptr)
        return -1;

    int numBytes=avpicture_get_size(pixelFormat, width,
                                    height);
    auto *buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    avpicture_fill((AVPicture *)*frame, buffer, pixelFormat,
                   width, height);

    return 0;
}

std::pair<int, int> get_resolution(std::string res)
{
    if(res == "1080p")
        return {1920, 1080};
    else if(res == "720p")
        return {1280, 720};
    else if(res == "480p")
        return {640, 480};
    else if(res == "360p")
        return {480, 360};
    else if(res == "240p")
        return {426, 240};


    std::cout << "Wrong resolution settings!" << std::endl;
    return {};
}

int main(int argc, char *argv[]) {

    std::string filename = argv[1];
    std::string resolution = argv[2];

    decoder video_decoder;
    metadata m_data;

    std::cout << "Filename: "<< filename << std::endl;

    if(video_decoder.init(filename, m_data) == -1)
        return 0;

    metadata m_data_target = m_data;

    auto res = get_resolution(resolution);
    m_data_target.width_ = res.first;
    m_data_target.height_ = res.second;


    encoder video_encoder;
    video_encoder.init(filename, resolution, m_data_target);

    colorspace_converter c_converter_to_rgb;
    c_converter_to_rgb.init(PIX_FMT_YUV420P,
                            PIX_FMT_RGB24,
                            m_data.width_,
                            m_data.height_,
                            m_data.width_,
                            m_data.height_);

    colorspace_converter c_converter_to_yuv;
    c_converter_to_yuv.init(PIX_FMT_RGB24,
                            PIX_FMT_YUV420P,
                            m_data_target.width_,
                            m_data_target.height_,
                            m_data_target.width_,
                            m_data_target.height_);


    /* Init target rgb frame which will be the resized frame*/
    AVFrame *frame_rgb_converted = nullptr;
    if(init_frame(&frame_rgb_converted,
                  m_data_target.width_,
                  m_data_target.height_,
                  PIX_FMT_RGB24) == -1)
    {
        std::cout << "Could not init target RGB frame!" << std::endl;
        return 0;
    }


    int stream_id = 0;
    int result = 0;
    int frame_count = 0;

    do
    {
        AVFrame *frame = nullptr;
        int line_size = 0;
        result = video_decoder.get_next_frame(stream_id, &frame, line_size);

        if(stream_id == m_data.video_stream_id_)
        {
            if(frame)
            {
                frame_count++;

                AVFrame *frame_rgb = c_converter_to_rgb.convert(frame);

                // save_frame(frame_rgb_converted, m_data_target.width_, m_data_target.height_, frame_count);
                // save_frame_txt(frame_rgb_converted, m_data_target.width_, m_data_target.height_, frame_count);

                resize(frame_rgb->data[0],
                       frame_rgb_converted->data[0],
                       m_data.width_,
                       m_data.height_,
                       m_data_target.width_,
                       m_data_target.height_);


                AVFrame *frame_yuv = c_converter_to_yuv.convert(frame_rgb_converted);

                int encoded = video_encoder.encode_frame(frame_yuv, frame_count);
                if(encoded == -1)
                    std::cout << "Could not encode frame!" << frame_count << std::endl;
            }
        }

    } while(result == 0);

    video_encoder.finalize();

    return 0;
}
