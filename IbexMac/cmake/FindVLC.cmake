# - Try to find VLC
# Once done this will define
#
# VLC_FOUND - system has VLC
# VLC_INCLUDE_DIRS - the VLC include directory
# VLC_LIBRARIES - Link these to use VLC
# VLC_DEFINITIONS - Compiler switches required for using VLC
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
      ~/Downloads/vlc/build/VLC.app/Contents/MacOS/include
      ~/Documents/vlc/build/VLC.app/Contents/MacOS/include
      ~/Downloads/vlc/build//vlc_install_dir/include
      /Applications/VLC.app/Contents/MacOS/include
      ~/Applications/VLC.app/Contents/MacOS/include
  )

find_path(VLC_LIBRARY_DIR
    NAMES
	libvlc.5.dylib libvlccore.5.dylib
#      libvlc.dylib libvlccore.dylib
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ~/Downloads/vlc/build/VLC.app/Contents/MacOS/lib
      ~/Documents/vlc/build/VLC.app/Contents/MacOS/lib
      ~/Downloads/vlc/build//vlc_install_dir/lib/
      /Applications/VLC.app/Contents/MacOS/lib
      ~/Applications/VLC.app/Contents/MacOS/lib
  )

  find_library(VLC_LIBRARY
    NAMES
        libvlc.5.dylib libvlccore.5.dylib
#	libvlc.dylib libvlccore.dylib
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ~/Downloads/vlc/build/VLC.app/Contents/MacOS/lib
      ~/Documents/vlc/build/VLC.app/Contents/MacOS/lib
      ~/Downloads/vlc/build//vlc_install_dir/lib/
      /Applications/VLC.app/Contents/MacOS/lib
      ~/Applications/VLC.app/Contents/MacOS/lib
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
      message(STATUS "Found VLC: ${VLC_LIBRARIES}")
    endif (NOT VLC_FIND_QUIETLY)
  else (VLC_FOUND)
    if (VLC_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find VLC")
    endif (VLC_FIND_REQUIRED)
  endif (VLC_FOUND)

  # show the VLC_INCLUDE_DIRS and VLC_LIBRARIES variables only in the advanced view
  mark_as_advanced(VLC_INCLUDE_DIRS VLC_LIBRARIES)

endif (VLC_LIBRARIES AND VLC_INCLUDE_DIRS AND VLC_LIBRARY_DIR)

