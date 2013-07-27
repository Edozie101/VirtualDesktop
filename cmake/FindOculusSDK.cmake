# - Try to find OculusSDK
# Once done this will define
#
# OCULUS_FOUND - system has OculusSDK
# OCULUS_INCLUDE_DIRS - the OculusSDK include directory
# OCULUS_LIBRARIES - Link these to use OculusSDK
# OCULUS_DEFINITIONS - Compiler switches required for using OculusSDK
#
# Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
if (OCULUS_LIBRARIES AND OCULUS_INCLUDE_DIRS AND OCULUS_LIBRARY_DIR)
  # in cache already
  set(OCULUS_FOUND TRUE)
else (OCULUS_LIBRARIES AND OCULUS_INCLUDE_DIRS AND OCULUS_LIBRARY_DIR)

  find_path(OCULUS_INCLUDE_DIR
    NAMES
      OVR.h OVRVersion.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      ~/OculusSDK/LibOVR/Include
      ~/Downloads/OculusSDK/LibOVR/Include
      ../OculusSDK/LibOVR/Include
      ../../OculusSDK/LibOVR/Include
  )

find_path(OCULUS_LIBRARY_DIR
    NAMES
      libovr.a
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

  find_library(OCULUS_LIBRARY
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

  if (OCULUS_LIBRARY)
    set(OCULUS_FOUND TRUE)
  endif (OCULUS_LIBRARY)

  set(OCULUS_INCLUDE_DIRS
    ${OCULUS_INCLUDE_DIR}
  )

  if (OCULUS_FOUND)
    set(OCULUS_LIBRARIES
      ${OCULUS_LIBRARIES}
      ${OCULUS_LIBRARY}
    )
  endif (OCULUS_FOUND)

  if (OCULUS_INCLUDE_DIRS AND OCULUS_LIBRARIES)
     set(OCULUS_FOUND TRUE)
  endif (OCULUS_INCLUDE_DIRS AND OCULUS_LIBRARIES)

  if (OCULUS_FOUND)
    if (NOT OCULUS_FIND_QUIETLY)
      message(STATUS "Found OculusSDK: ${OCULUS_LIBRARIES}")
    endif (NOT OCULUS_FIND_QUIETLY)
  else (OCULUS_FOUND)
    if (OCULUS_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OculusSDK")
    endif (OCULUS_FIND_REQUIRED)
  endif (OCULUS_FOUND)

  # show the OCULUS_INCLUDE_DIRS and OCULUS_LIBRARIES variables only in the advanced view
  mark_as_advanced(OCULUS_INCLUDE_DIRS OCULUS_LIBRARIES)

endif (OCULUS_LIBRARIES AND OCULUS_INCLUDE_DIRS AND OCULUS_LIBRARY_DIR)

