set(ibex_windows_SOURCES 
    TextRenderer.cpp TextRenderer.h
    stb_truetype.cpp stb_truetype.h
    Window.cpp  Window.h)

add_library(ibex_windows ${ibex_windows_SOURCES})
