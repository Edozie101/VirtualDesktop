include_directories(/usr/local/include ../../../sixenseSDK_linux_OSX/include)
link_directories(/usr/local/lib)

set(ibex_sixense_SOURCES sixense_controller.cpp sixense_controller.h)

add_library(ibex_sixense ${ibex_sixense_SOURCES})
