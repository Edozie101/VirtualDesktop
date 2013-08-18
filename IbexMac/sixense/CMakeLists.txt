include_directories(/usr/local/include ../../../sixenseSDK_linux_OSX/include)
link_directories(/usr/local/lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D=_USE_SIXENSE=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_SIXENSE=1")

set(ibex_sixense_SOURCES
sixense_controller.cpp sixense_controller.h
)

add_library(ibex_sixense ${ibex_sixense_SOURCES})
