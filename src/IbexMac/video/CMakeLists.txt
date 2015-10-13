set(ibex_video_SOURCES 
IbexVideoPlayer.mm IbexVideoPlayer.h
VLCVideoPlayer.cpp VLCVideoPlayer.h
#VideoPlayer.cpp  VideoPlayer.h
)

add_library(ibex_video ${ibex_video_SOURCES})
