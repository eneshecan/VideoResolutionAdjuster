extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "decoder.h"

#include <iostream>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

void SaveFrame(uint8_t *frame, int line_size, int width, int height, int iFrame) {
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
        fwrite(frame + y*line_size, 1, (size_t)width*3, pFile);

    fclose(pFile);
}

int main(int argc, char *argv[]) {

    decoder video_decoder;
    metadata m_data;

    if(video_decoder.init(argv[1], m_data) == -1)
        return 0;

    int stream_id = 0;
    int result = 0;
    int frame_count = 0;

    do
    {
        uint8_t *frame_rgb = nullptr;
        int line_size = 0;
        result = video_decoder.get_next_frame(stream_id, &frame_rgb, line_size);

        if(stream_id == m_data.video_stream_id_)
        {
            if(frame_rgb)
            {
                frame_count++;
                SaveFrame(frame_rgb, line_size, m_data.width_, m_data.height_, frame_count);
            }
        }

    } while(result == 0);

    return 0;
}
