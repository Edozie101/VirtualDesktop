# - Try to find OGRE
# Once done this will define
#
# OGRE_FOUND - system has OGRE
# OGRE_INCLUDE_DIRS - the OGRE include directory
# OGRE_LIBRARIES - Link these to use OGRE
# OGRE_DEFINITIONS - Compiler switches required for using OGRE
#
# Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

#if (OGRE_INCLUDE_DIR)
  # in cache already
  #set(OGRE_FOUND TRUE)
#else (OGRE_INCLUDE_DIR)

  find_path(OGRE_INCLUDE_DIR
    NAMES
      Ogre.h
    PATHS
      /usr/include
      /usr/include/OGRE
      /usr/local/include
      /usr/local/include/OGRE
      /opt/local/include
      /sw/include
      ~/Documents/OgreSDK/include/OGRE
      ~/Downloads/OgreSDK/include/OGRE
      ~/Downloads/OgreSDK/OgreMain/include
  )
  find_path(OGRE_OIS_INCLUDE_DIR
    NAMES
      OIS.h
    PATHS
      /usr/include
      /usr/include/OIS
      /usr/local/include
      /usr/local/include/OIS
      /opt/local/include
      /sw/include
      ~/Documents/OgreSDK/include/OIS
      ~/Downloads/OgreSDK/include/OIS
      ~/Downloads/OgreSDK/Dependencies/include/OIS
  )

  
  find_path(OGRE_LIBS_DIR
    NAMES
      Ogre.framework
    PATHS
      /usr/include
      /usr/include/OGRE
      /usr/local/include
      /usr/local/include/OGRE
      /opt/local/include
      /sw/include
      ~/Documents/OgreSDK/lib/macosx/Release
      ~/Downloads/OgreSDK/lib/macosx/Release
      ~/Downloads/build_ogre/lib/macosx/Release
  )


  if (OGRE_INCLUDE_DIR)
    set(OGRE_FOUND TRUE)
  endif (OGRE_INCLUDE_DIR)


   INCLUDE_DIRECTORIES ( ${OGRE_FRAMEWORK_DIR}  ${OGRE_INCLUDE_DIR} ${OGRE_OIS_INCLUDE_DIR}
   
   ${OGRE_INCLUDE_DIR}/Overlay
   ${OGRE_INCLUDE_DIR}/Paging
   ${OGRE_INCLUDE_DIR}/Plugins
   ${OGRE_INCLUDE_DIR}/Property
   ${OGRE_INCLUDE_DIR}/RTShaderSystem
   ${OGRE_INCLUDE_DIR}/RenderSystems
    ${OGRE_INCLUDE_DIR}/RenderSystems/GL
   ${OGRE_INCLUDE_DIR}/Terrain
   ${OGRE_INCLUDE_DIR}/Threading
   ${OGRE_INCLUDE_DIR}/Volume
   ${OGRE_INCLUDE_DIR}/OSX

    )
    
    set(OGRE_INCLUDE_DIR ${OGRE_INCLUDE_DIR} ${OGRE_OIS_INCLUDE_DIR}
       ${OGRE_INCLUDE_DIR}/Overlay
   ${OGRE_INCLUDE_DIR}/Paging
   ${OGRE_INCLUDE_DIR}/Plugins
   ${OGRE_INCLUDE_DIR}/Property
   ${OGRE_INCLUDE_DIR}/RTShaderSystem
   ${OGRE_INCLUDE_DIR}/RenderSystems
   ${OGRE_INCLUDE_DIR}/RenderSystems/GL
   ${OGRE_INCLUDE_DIR}/Terrain
   ${OGRE_INCLUDE_DIR}/Threading
   ${OGRE_INCLUDE_DIR}/Volume
   ${OGRE_INCLUDE_DIR}/OSX
    )

   SET(OGRE_STATIC_LIBS
libOgreMainStatic
libOgreOverlayStatic
libOgrePagingStatic
libOgrePropertyStatic
libOgreRTShaderSystemStatic
libOgreTerrainStatic
libOgreVolumeStatic
libPlugin_BSPSceneManagerStatic
libPlugin_OctreeSceneManagerStatic
libPlugin_OctreeZoneStatic
libPlugin_PCZSceneManagerStatic
libPlugin_ParticleFXStatic
libRenderSystem_GLStatic
   )
   
   link_directories(${OGRE_LIBS_DIR} ~/Downloads/cabalistic-ogredeps-87596e62bf28/build/ogredeps/lib)#~/Downloads/OgreDependencies/lib/Release)
   SET(OGRE_LIBS
        Ogre.framework
        #OgreProperty.framework
        #OgreVolume.framework
        Plugin_OctreeZone.framework         
        RenderSystem_GL.framework
        #OgreOverlay.framework
        OgreRTShaderSystem.framework
        #Plugin_BSPSceneManager.framework
        #Plugin_PCZSceneManager.framework
        #OgrePaging.framework
        #OgreTerrain.framework               
        #Plugin_OctreeSceneManager.framework
        #Plugin_ParticleFX.framework
    )
    
    set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} ${OGRE_LIBS_DIR})
    find_library(OGRE_LIBRARY Ogre)
    find_library(OGRE_PROPERTY_LIBRARY OgreProperty)
    find_library(OGRE_VOLUME_LIBRARY OgreVolume)
    find_library(OGRE_OVERLAY_LIBRARY OgreOverlay)
    find_library(OGRE_PAGING_LIBRARY OgrePaging)
    find_library(OGRE_TERRAIN_LIBRARY OgreTerrain)
    find_library(OGRE_RTSHADER_SYSTEM_LIBRARY OgreRTShaderSystem)
    find_library(RENDERSYSTEM_GL_LIBRARY RenderSystem_GL)

   MARK_AS_ADVANCED (OGRE_LIBRARY
                     OGRE_PROPERTY_LIBRARY
                     OGRE_VOLUME_LIBRARY
                     RENDERSYSTEM_GL_LIBRARY
                     OGRE_RTSHADER_SYSTEM_LIBRARY
                     )
   SET(OGRE_LIBS 
   
${OGRE_LIBRARY}
#    ${OGRE_PROPERTY_LIBRARY}
#    ${OGRE_VOLUME_LIBRARY}
    ${RENDERSYSTEM_GL_LIBRARY}
    ${OGRE_OVERLAY_LIBRARY}
#    ${OGRE_PAGING_LIBRARY}
    ${OGRE_TERRAIN_LIBRARY}
    ${OGRE_RTSHADER_SYSTEM_LIBRARY}
    OIS
   )


  if (OGRE_INCLUDE_DIR)
     set(OGRE_FOUND TRUE)
  endif (OGRE_INCLUDE_DIR)

  if (OGRE_FOUND)
    #if (NOT OGRE_FIND_QUIETLY)
      message(STATUS "Found OGRE: ${OGRE_INCLUDE_DIR}")
      message(STATUS "Found OGRE: ${OGRE_LIBS_DIR}")
    #endif (NOT OGRE_FIND_QUIETLY)
  else (OGRE_FOUND)
#    if (OGRE_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OGRE")
#    endif (OGRE_FIND_REQUIRED)
  endif (OGRE_FOUND)

  # show the OGRE_INCLUDE_DIRS and OGRE_LIBRARIES variables only in the advanced view
#  mark_as_advanced(OGRE_INCLUDE_DIR)

#endif (OGRE_INCLUDE_DIR)

