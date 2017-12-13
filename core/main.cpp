extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "decoder.h"
#include "encoder.h"


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

    if(video_decoder.init(argv[1], m_data) == -1)
        return 0;

    encoder video_encoder;
    video_encoder.init(argv[1], m_data);

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
                int encoded = video_encoder.encode_frame(frame, frame_count);
                if(encoded == -1)
                    std::cout << "Could not encode frame " << frame_count << std::endl;

                //SaveFrame(frame, m_data.width_, m_data.height_, frame_count);
            }
        }

    } while(result == 0);

    video_encoder.finalize();

    return 0;
}
