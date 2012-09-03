project(ibex)

cmake_minimum_required(VERSION 2.8)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBJPEG=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_LIBJPEG=1")

set(SRC ibex.cpp glm/glm_util.c glm/glm.c glm/glmimg.c glm/glmimg_png.c glm/glmimg_jpg.c)
add_executable(ibex ${SRC})
target_link_libraries(ibex GL GLU glut GLEW X11 Xfixes Xdamage Xrender Xcomposite Xxf86vm Xi Xext jpeg)

install (FILES lcd_monitor.obj lcd_monitor.mtl DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
install (DIRECTORY humus-skybox DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
