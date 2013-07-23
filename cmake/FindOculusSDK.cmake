# - Try to find OculusSDK
# Once done this will define
#
# OCULUSSDK_FOUND - system has OculusSDK
# OCULUSSDK_INCLUDE_DIRS - the OculusSDK include directory
# OCULUSSDK_LIBRARIES - Link these to use OculusSDK
# OCULUSSDK_DEFINITIONS - Compiler switches required for using OculusSDK
#
# Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
if (OCULUSSDK_LIBRARIES AND OCULUSSDK_INCLUDE_DIRS)
  # in cache already
  set(OCULUSSDK_FOUND TRUE)
else (OCULUSSDK_LIBRARIES AND OCULUSSDK_INCLUDE_DIRS)

  find_path(OCULUSSDK_INCLUDE_DIR
    NAMES
      OVR.h OVRVersion.h
    PATHS
      /usr/include
      /usr/include/irrlicht
      /usr/local/include
      /usr/local/include/irrlicht
      /opt/local/include
      /sw/include
      ~/OculusSDK/LibOVR/Include
      ~/Downloads/OculusSDK/LibOVR/Include
      ../OculusSDK/LibOVR/Include
      ../../OculusSDK/LibOVR/Include
  )

  find_library(OCULUSSDK_LIBRARY
    NAMES
        ovr
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ~/OculusSDK/LibOVR/Lib/Linux/Release/x86_64
      ~/Downloads/OculusSDK/LibOVR/Lib/Linux/Release/x86_64
      ../OculusSDK/LibOVR/Lib/Linux/Release/x86_64
      ../../OculusSDK/LibOVR/Lib/Linux/Release/x86_64
  )

  if (OCULUSSDK_LIBRARY)
    set(OCULUSSDK_FOUND TRUE)
  endif (OCULUSSDK_LIBRARY)

  set(OCULUSSDK_INCLUDE_DIRS
    ${OCULUSSDK_INCLUDE_DIR}
  )

  if (OCULUSSDK_FOUND)
    set(OCULUSSDK_LIBRARIES
      ${OCULUSSDK_LIBRARIES}
      ${OCULUSSDK_LIBRARY}
    )
  endif (OCULUSSDK_FOUND)

  if (OCULUSSDK_INCLUDE_DIRS AND OCULUSSDK_LIBRARIES)
     set(OCULUSSDK_FOUND TRUE)
  endif (OCULUSSDK_INCLUDE_DIRS AND OCULUSSDK_LIBRARIES)

  if (OCULUSSDK_FOUND)
    if (NOT OCULUSSDK_FIND_QUIETLY)
      message(STATUS "Found OculusSDK: ${OCULUSSDK_LIBRARIES}")
    endif (NOT OCULUSSDK_FIND_QUIETLY)
  else (OCULUSSDK_FOUND)
    if (OCULUSSDK_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OculusSDK")
    endif (OCULUSSDK_FIND_REQUIRED)
  endif (OCULUSSDK_FOUND)

  # show the OCULUSSDK_INCLUDE_DIRS and OCULUSSDK_LIBRARIES variables only in the advanced view
  mark_as_advanced(OCULUSSDK_INCLUDE_DIRS OCULUSSDK_LIBRARIES)

endif (OCULUSSDK_LIBRARIES AND OCULUSSDK_INCLUDE_DIRS)

