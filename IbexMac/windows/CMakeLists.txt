set(ibex_windows_SOURCES 
    ApplicationLauncher.cpp ApplicationLauncher.h
    TextRenderer.cpp TextRenderer.h
    stb_truetype.cpp stb_truetype.h
    Window.cpp  Window.h)

add_library(ibex_windows ${ibex_windows_SOURCES})
