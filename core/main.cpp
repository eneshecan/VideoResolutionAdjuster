extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "decoder.h"
#include "encoder.h"
#include "colorspace_converter.h"

#include <fstream>
void resize(uint8_t* input, uint8_t* output, int sourceWidth, int sourceHeight, int targetWidth, int targetHeight) 
{    
    int  b, c, d, x, y, index;
    uint8_t a;
    float x_ratio = ((float)(sourceWidth - 1)) / targetWidth;
    float y_ratio = ((float)(sourceHeight - 1)) / targetHeight;
    float x_diff, y_diff, blue, red, green ;
    uint8_t offset = 0 ;

    for (int i = 0; i < targetHeight; i++) 
    {
        for (int j = 0; j < targetWidth; j++) 
        {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            //x_diff = (x_ratio * j) - x ;
            //y_diff = (y_ratio * i) - y ;
            index = (y * sourceWidth + x)*3 ;                
            a = input[index] ;
            
            // blue element
            blue = a+2; 
            // green element
            green = a+1;

            // red element
            red = a;

            output [offset++] =red;
	    output [offset++] =green;
	    output [offset++] =blue;
	    
	    
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
void SaveFrame(AVFrame *frame, int width, int height, int frame_count) {
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

int main(int argc, char *argv[]) {

    decoder video_decoder;
    metadata m_data;

    std::cout << argv[1] << std::endl;

    if(video_decoder.init(argv[1], m_data) == -1)
        return 0;

    encoder video_encoder;
    video_encoder.init(argv[1], m_data);

    colorspace_converter c_converter_to_rgb;
    c_converter_to_rgb.init(PIX_FMT_YUV420P, PIX_FMT_RGB24, m_data.width_, m_data.height_, m_data.width_, m_data.height_);

    colorspace_converter c_converter_to_yuv;
    c_converter_to_yuv.init(PIX_FMT_RGB24, PIX_FMT_YUV420P, m_data.width_, m_data.height_, m_data.width_, m_data.height_);

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

                save_frame_txt(frame_rgb, m_data.width_, m_data.height_, frame_count);
                // Resizer will operate here

                AVFrame *frame_yuv = c_converter_to_yuv.convert(frame_rgb);

                int encoded = video_encoder.encode_frame(frame_yuv, frame_count);
                if(encoded == -1)
                    std::cout << "Could not encode frame " << frame_count << std::endl;
            }
        }

    } while(result == 0);

    video_encoder.finalize();

    return 0;
}
