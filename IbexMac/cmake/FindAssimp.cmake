# - Try to find Assimp
# Once done this will define
#
# ASSIMP_FOUND - system has Assimp
# ASSIMP_INCLUDE_DIRS - the Assimp include directory
# ASSIMP_LIBRARIES - Link these to use Assimp
# ASSIMP_DEFINITIONS - Compiler switches required for using Assimp
#
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
if (ASSIMP_LIBRARIES AND ASSIMP_INCLUDE_DIRS AND ASSIMP_LIBRARY_DIR)
  # in cache already
  set(ASSIMP_FOUND TRUE)
else (ASSIMP_LIBRARIES AND ASSIMP_INCLUDE_DIRS AND ASSIMP_LIBRARY_DIR)

    message("SEARCING for assimp")
  find_path(ASSIMP_INCLUDE_DIR
    NAMES
      assimp/Importer.hpp
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

find_path(ASSIMP_LIBRARY_DIR
    NAMES
      libassimp.dylib
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  find_library(ASSIMP_LIBRARY
    NAMES
        assimp
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (ASSIMP_LIBRARY)
    set(ASSIMP_FOUND TRUE)
  endif (ASSIMP_LIBRARY)

  set(ASSIMP_INCLUDE_DIRS
    ${ASSIMP_INCLUDE_DIR}
  )

  if (ASSIMP_FOUND)
    set(ASSIMP_LIBRARIES
      ${ASSIMP_LIBRARIES}
      ${ASSIMP_LIBRARY}
    )
  endif (ASSIMP_FOUND)

  if (ASSIMP_INCLUDE_DIRS AND ASSIMP_LIBRARIES)
     set(ASSIMP_FOUND TRUE)
  endif (ASSIMP_INCLUDE_DIRS AND ASSIMP_LIBRARIES)

  if (ASSIMP_FOUND)
    if (NOT ASSIMP_FIND_QUIETLY)
      message(STATUS "Found Assimp: ${ASSIMP_LIBRARIES}")
    endif (NOT ASSIMP_FIND_QUIETLY)
  else (ASSIMP_FOUND)
    if (ASSIMP_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Assimp")
    endif (ASSIMP_FIND_REQUIRED)
  endif (ASSIMP_FOUND)

  # show the ASSIMP_INCLUDE_DIRS and ASSIMP_LIBRARIES variables only in the advanced view
  mark_as_advanced(ASSIMP_INCLUDE_DIRS ASSIMP_LIBRARIES)

endif (ASSIMP_LIBRARIES AND ASSIMP_INCLUDE_DIRS AND ASSIMP_LIBRARY_DIR)

