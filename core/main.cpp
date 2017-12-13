extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "decoder.h"

void SaveFrame(AVFrame *frame, int width, int height, int iFrame) {
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
        fwrite(frame->data[0] + y* frame->linesize[0], 1, (size_t)width*3, pFile);

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
        AVFrame *frame_rgb = nullptr;
        int line_size = 0;
        result = video_decoder.get_next_frame(stream_id, &frame_rgb, line_size);

        if(stream_id == m_data.video_stream_id_)
        {
            if(frame_rgb)
            {
                frame_count++;
                SaveFrame(frame_rgb, m_data.width_, m_data.height_, frame_count);
            }
        }

    } while(result == 0);

    return 0;
}
