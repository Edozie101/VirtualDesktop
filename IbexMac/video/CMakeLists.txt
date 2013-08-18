find_package(OpenCV)

IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(APPLICATIONSERVICES_LIBRARY ApplicationServices)
   FIND_LIBRARY(VIDEODECODEACCELERATION_LIBRARY VideoDecodeAcceleration)
   FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo)
   FIND_LIBRARY(IOKIT_LIBRARY IOKit)
   FIND_LIBRARY(OPENAL_LIBRARY OpenAL)
   FIND_LIBRARY(QUARTZ_CORE_LIBRARY QuartzCore)
   FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
   FIND_LIBRARY(CARBON_LIBRARY Carbon)
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
   FIND_LIBRARY(APPKIT_LIBRARY AppKit)

   MARK_AS_ADVANCED (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY
                     APPLICATIONSERVICES_LIBRARY
                     VIDEODECODEACCELERATION_LIBRARY
                     COREVIDEO_LIBRARY
                     IOKIT_LIBRARY
                     OPENAL_LIBRARY
                     QUARTZ_CORE_LIBRARY
                     COREFOUNDATION_LIBRARY
                     CARBON_LIBRARY
                     APPKIT_LIBRARY)

   SET(EXTRA_LIBS ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY} 
                    ${APPLICATIONSERVICES_LIBRARY}
                    ${VIDEODECODEACCELERATION_LIBRARY}
                    ${COREVIDEO_LIBRARY}
                    ${IOKIT_LIBRARY}
                    ${OPENAL_LIBRARY}
                    ${QUARTZ_CORE_LIBRARY}
                    ${COREFOUNDATION_LIBRARY}
                    ${CARBON_LIBRARY}
                    ${APPKIT_LIBRARY}
   )
ENDIF (APPLE)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O0 -ggdb -DHAVE_LIBJPEG=1 ${OCULUS_INCLUDE_FLAGS} ${VLC_INCLUDE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g  -O0 -ggdb -DHAVE_LIBJPEG=1 ${OCULUS_INCLUDE_FLAGS} ${VLC_INCLUDE_FLAGS}")


set(ibex_video_SOURCES 
IbexVideoPlayer.mm IbexVideoPlayer.h
VLCVideoPlayer.cpp VLCVideoPlayer.h
#VideoPlayer.cpp  VideoPlayer.h
)

add_library(ibex_video ${ibex_video_SOURCES})
