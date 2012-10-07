project(ibex)

cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
SET(CMAKE_MODULE_PATH "/usr/share/OGRE/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(OpenGL REQUIRED)	
find_package(GLUT REQUIRED)
find_package(Irrlicht)
find_package(OGRE)
find_package(OIS)

if( OGRE_FOUND )
  add_definitions( -DENABLE_OGRE3D )
  #set(OGRE_INCLUDE_FLAGS "-I${OGRE_INCLUDE_DIRS}")
  set(OGRE_PLUGIN_FILES "ogre3d_plugin/BaseApplication.cpp" "ogre3d_plugin/TutorialApplication.cpp")
  include_directories( ${OIS_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_SAMPLES_INCLUDEPATH}
)
endif( OGRE_FOUND )

if( IRRLICHT_FOUND )  
  add_definitions( -DENABLE_IRRLICHT )
  set(IRRLICHT_INCLUDE_FLAGS "-I${IRRLICHT_INCLUDE_DIRS}")
  set(IRRLICHT_PLUGIN_FILES "irrlicht_plugin/irrlicht_plugin.cpp")
endif( IRRLICHT_FOUND )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DHAVE_LIBJPEG=1 ${IRRLICHT_INCLUDE_FLAGS} ${OGRE_INCLUDE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DHAVE_LIBJPEG=1 ${IRRLICHT_INCLUDE_FLAGS} ${OGRE_INCLUDE_FLAGS}")

set(SRC 
ibex.cpp utils.cpp distortions.cpp opengl_helpers.cpp opengl_setup_x11.cpp
RendererPlugin.cpp
simpleworld_plugin/SimpleWorldRendererPlugin.cpp
iphone_orientation_plugin/iphone_orientation_listener.cpp
glm/glm_util.c glm/glm.c glm/glmimg.c glm/glmimg_png.c glm/glmimg_jpg.c)

add_executable(ibex ${SRC} ${IRRLICHT_PLUGIN_FILES} ${OGRE_PLUGIN_FILES})
target_link_libraries(ibex GL GLU glut GLEW X11 Xfixes Xdamage Xrender Xcomposite Xxf86vm Xi Xext jpeg rt ${IRRLICHT_LIBRARY} ${OGRE_LIBRARIES} ${OIS_LIBRARIES})

install (DIRECTORY resources DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
