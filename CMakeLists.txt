project(ibex)

cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Irrlicht)

if( IRRLICHT_FOUND )
  add_definitions( -DENABLE_IRRLICHT )
  set(IRRLICHT_INCLUDE_FLAGS "-I${IRRLICHT_INCLUDE_DIRS}")
  set(IRRLICHT_PLUGIN_FILES "irrlicht_plugin/irrlicht_plugin.cpp")
endif( IRRLICHT_FOUND )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBJPEG=1 ${IRRLICHT_INCLUDE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_LIBJPEG=1 ${IRRLICHT_INCLUDE_FLAGS}")

set(SRC 
ibex.cpp utils.cpp distortions.cpp opengl_helpers.cpp opengl_setup_x11.cpp
RendererPlugin.cpp
simpleworld_plugin/SimpleWorldRendererPlugin.cpp
iphone_orientation_plugin/iphone_orientation_listener.cpp
glm/glm_util.c glm/glm.c glm/glmimg.c glm/glmimg_png.c glm/glmimg_jpg.c)

add_executable(ibex ${SRC} ${IRRLICHT_PLUGIN_FILES})
target_link_libraries(ibex GL GLU glut GLEW X11 Xfixes Xdamage Xrender Xcomposite Xxf86vm Xi Xext jpeg rt ${IRRLICHT_LIBRARY})

install (DIRECTORY resources DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
