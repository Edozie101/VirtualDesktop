project(ibex)

cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
SET(CMAKE_MODULE_PATH "/usr/share/OGRE/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(OpenGL REQUIRED)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Linux specific code
    SET(OperatingSystem "Linux")

find_package(GLUT REQUIRED)
find_package(OculusSDK)
find_package(OpenAL)
find_package(Irrlicht)
find_package(OGRE)
find_package(OIS)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.53.0 COMPONENTS filesystem system)

if( OGRE_FOUND AND OGRE )
  add_definitions( -DENABLE_OGRE3D )
  #set(OGRE_INCLUDE_FLAGS "-I${OGRE_INCLUDE_DIRS}")
  set(OGRE_PLUGIN_FILES "ogre3d_plugin/BaseApplication.cpp" "ogre3d_plugin/TutorialApplication.cpp" "ogre3d_plugin/DotSceneLoader.cpp")
  include_directories( ${OIS_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_SAMPLES_INCLUDEPATH}
)
endif( OGRE_FOUND AND OGRE )

if( OCULUS_FOUND )
  include_directories( ${OCULUS_INCLUDE_DIRS} )
endif( OCULUS_FOUND )

if( IRRLICHT_FOUND AND IRRLICHT )  
  add_definitions( -DENABLE_IRRLICHT )
  set(IRRLICHT_INCLUDE_FLAGS "-I${IRRLICHT_INCLUDE_DIRS}")
  set(IRRLICHT_PLUGIN_FILES "irrlicht_plugin/irrlicht_plugin.cpp")
endif( IRRLICHT_FOUND AND IRRLICHT )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O0 -ggdb -DHAVE_LIBJPEG=1 ${IRRLICHT_INCLUDE_FLAGS} ${OGRE_INCLUDE_FLAGS} ${OCULUS_INCLUDE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g  -O0 -ggdb -DHAVE_LIBJPEG=1 ${IRRLICHT_INCLUDE_FLAGS} ${OGRE_INCLUDE_FLAGS} ${OCULUS_INCLUDE_FLAGS}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_subdirectory(glm)
add_subdirectory(windows)
add_subdirectory(filesystem)
add_subdirectory(video)
add_subdirectory(sixense)
add_subdirectory(oculus)
add_subdirectory(x11)

include_directories(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${Boost_INCLUDE_DIR}
 )
link_directories(${Boost_LIBRARY_DIR} ${OCULUS_LIBRARY_DIR})

set(SRC 
RendererPlugin.cpp RendererPlugin.h 
distortions.cpp distortions.h 
ibex.cpp ibex.h 
opengl_helpers.cpp opengl_helpers.h
opengl_setup_x11.cpp opengl_setup_x11.h
utils.cpp utils.h
simpleworld_plugin/SimpleWorldRendererPlugin.cpp simpleworld_plugin/SimpleWorldRendererPlugin.h
iphone_orientation_plugin/iphone_orientation_listener.cpp iphone_orientation_plugin/iphone_orientation_listener.h
)

set(RESOURCES README.md COPYING AUTHORS)

add_executable(ibex ${SRC} ${IRRLICHT_PLUGIN_FILES} ${OGRE_PLUGIN_FILES})
target_link_libraries(ibex 
ibex_windows ibex_filesystem ibex_x11 ibex_video ibex_glm ibex_sixense ibex_oculus
GL GLU glut GLEW
vlc
opencv_core opencv_highgui opencv_video opencv_contrib opencv_ts opencv_legacy 
openal
ovr
X11 Xfixes Xdamage Xrandr Xrender Xcomposite Xxf86vm Xi Xext Xinerama udev pthread
jpeg rt
${Boost_LIBRARIES}
#${OCULUS_LIBRARY}
${IRRLICHT_LIBRARY} ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_RenderSystem_GL_LIBRARIES}
${OGRE_Terrain_LIBRARIES}
${OGRE_ParticleFX_LIBRARIES}
${OGRE_BSPSceneManager_LIBRARIES}
${OGRE_CgProgramManager_LIBRARIES}
${OGRE_PCZSceneManager_LIBRARIES}
${OGRE_OctreeZone_LIBRARIES}
${OGRE_OctreeSceneManager_LIBRARIES}
tinyxml
)
 
set_target_properties( ibex PROPERTIES
RESOURCE "${RESOURCES}" )

install (DIRECTORY resources DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
