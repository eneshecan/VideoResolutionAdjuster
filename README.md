# VideoResolutionAdjuster
By lowering high resolution, 4K/8K videos will be played comfortably on old machines.

## Requirements
- FFMpeg version 2.8.11 or higher

## Run
- With the command `ffmpeg main.cpp <path_to_video> -lavutil -lavcodec -lavformat`
- Or by using CMakeLists.txt and modifying according to the library paths on local machine
