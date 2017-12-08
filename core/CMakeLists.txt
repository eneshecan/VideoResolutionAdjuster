cmake_minimum_required(VERSION 3.8 )
project(VideoResAdjuster C CXX)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif()

find_path( AVUTIL_INCLUDE_DIR libavutil/avutil.h
        HINTS
        $ENV{AVUTILDIR}
        PATH_SUFFIXES include
        PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include
        /opt/local/include
        /opt/csw/include
        /opt/include
        /mingw/include
        )

message( "AVUTIL_INCLUDE_DIR is ${AVUTIL_INCLUDE_DIR}" )

find_library( AVUTIL_LIBRARY
        NAMES avutil
        HINTS
        $ENV{AVUTILDIR}
        PATH_SUFFIXES lib64 lib bin
        PATHS
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        /mingw
        )

message( "AVUTIL_LIBRARY is ${AVUTIL_LIBRARY}" )

set( AVUTIL_FOUND "YES" )

find_path( AVFORMAT_INCLUDE_DIR libavformat/avformat.h
        HINTS
        $ENV{AVFORMATDIR}
        PATH_SUFFIXES include
        PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include
        /opt/local/include
        /opt/csw/include
        /opt/include
        /mingw/include
        )

message( "AVFORMAT_INCLUDE_DIR is ${AVFORMAT_INCLUDE_DIR}" )

find_library( AVFORMAT_LIBRARY
        NAMES avformat
        HINTS
        $ENV{AVFORMATDIR}
        PATH_SUFFIXES lib64 lib bin
        PATHS
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        /mingw
        )

message( "AVFORMAT_LIBRARY is ${AVFORMAT_LIBRARY}" )

set( AVFORMAT_FOUND "YES" )

set ( AV_CODEC_LIBRARY "/usr/lib/x86_64-linux-gnu/libavcodec-ffmpeg.so")

set ( SW_SCALE_LIBRARY "/usr/lib/x86_64-linux-gnu/libswscale-ffmpeg.so")

set(
        FFMPEG_LIBRARIES
        ${AVFORMAT_LIBRARY}
        ${AV_CODEC_LIBRARY}
        ${SW_SCALE_LIBRARY}
        ${AVUTIL_LIBRARY}
)

set(SOURCE_FILES main.cpp decoder.cpp decoder.h)

add_executable(VideoResAdjuster ${SOURCE_FILES})

target_link_libraries(VideoResAdjuster ${FFMPEG_LIBRARIES})

