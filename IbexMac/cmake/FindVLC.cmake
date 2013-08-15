# - Try to find OculusSDK
# Once done this will define
#
# VLC_FOUND - system has OculusSDK
# VLC_INCLUDE_DIRS - the OculusSDK include directory
# VLC_LIBRARIES - Link these to use OculusSDK
# VLC_DEFINITIONS - Compiler switches required for using OculusSDK
#
# Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
if (VLC_LIBRARIES AND VLC_INCLUDE_DIRS AND VLC_LIBRARY_DIR)
  # in cache already
  set(VLC_FOUND TRUE)
else (VLC_LIBRARIES AND VLC_INCLUDE_DIRS AND VLC_LIBRARY_DIR)

  find_path(VLC_INCLUDE_DIR
    NAMES
      vlc/vlc.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      /Applications/VLC.app/Contents/MacOS/include
      ~/Applications/VLC.app/Contents/MacOS/include
      ~/Downloads/vlc/build/VLC.app/Contents/MacOS/include
      ~/Documents/vlc/build/VLC.app/Contents/MacOS/include
  )

find_path(VLC_LIBRARY_DIR
    NAMES
      libvlc.dylib
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      /Applications/VLC.app/Contents/MacOS/lib
      ~/Applications/VLC.app/Contents/MacOS/lib
      ~/Downloads/vlc/build/VLC.app/Contents/MacOS/lib
      ~/Documents/vlc/build/VLC.app/Contents/MacOS/lib
  )

  find_library(VLC_LIBRARY
    NAMES
        libvlc.dylib
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      /Applications/VLC.app/Contents/MacOS/lib
      ~/Applications/VLC.app/Contents/MacOS/lib
      ~/Downloads/vlc/build/VLC.app/Contents/MacOS/lib
      ~/Documents/vlc/build/VLC.app/Contents/MacOS/lib
  )

  if (VLC_LIBRARY)
    set(VLC_FOUND TRUE)
  endif (VLC_LIBRARY)

  set(VLC_INCLUDE_DIRS
    ${VLC_INCLUDE_DIR}
  )

  if (VLC_FOUND)
    set(VLC_LIBRARIES
      ${VLC_LIBRARIES}
      ${VLC_LIBRARY}
    )
  endif (VLC_FOUND)

  if (VLC_INCLUDE_DIRS AND VLC_LIBRARIES)
     set(VLC_FOUND TRUE)
  endif (VLC_INCLUDE_DIRS AND VLC_LIBRARIES)

  if (VLC_FOUND)
    if (NOT VLC_FIND_QUIETLY)
      message(STATUS "Found OculusSDK: ${VLC_LIBRARIES}")
    endif (NOT VLC_FIND_QUIETLY)
  else (VLC_FOUND)
    if (VLC_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OculusSDK")
    endif (VLC_FIND_REQUIRED)
  endif (VLC_FOUND)

  # show the VLC_INCLUDE_DIRS and VLC_LIBRARIES variables only in the advanced view
  mark_as_advanced(VLC_INCLUDE_DIRS VLC_LIBRARIES)

endif (VLC_LIBRARIES AND VLC_INCLUDE_DIRS AND VLC_LIBRARY_DIR)

