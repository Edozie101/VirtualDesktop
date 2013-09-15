set(HEADER_FILES
    CfgFileManager.h
    GPUNormalMapManager.h
    MaterialManager.h  TextureManager.h
    DecalsManager.h
    Help.h
    Mesh.h
    Enums.h
    Hydrax.h
    Prerequisites.h
    GodRaysManager.h
    Image.h
    RttManager.h
    Noise/Noise.h
    Noise/Perlin/Perlin.h
    Noise/FFT/FFT.h
    Modules/Module.h
    Modules/ProjectedGrid/ProjectedGrid.h
    Modules/RadialGrid/RadialGrid.h
    Modules/SimpleGrid/SimpleGrid.h

)
set(SOURCE_FILES
    CfgFileManager.cpp
    Help.cpp
    Prerequisites.cpp
    DecalsManager.cpp
    Hydrax.cpp
    RttManager.cpp
    Enums.cpp
    Image.cpp
    TextureManager.cpp
    GodRaysManager.cpp
    MaterialManager.cpp
    GPUNormalMapManager.cpp
    Mesh.cpp
    Noise/Noise.cpp
    Noise/Perlin/Perlin.cpp
    Noise/FFT/FFT.cpp
    Modules/Module.cpp
    Modules/ProjectedGrid/ProjectedGrid.cpp
    Modules/RadialGrid/RadialGrid.cpp
    Modules/SimpleGrid/SimpleGrid.cpp
)

include_directories (.)

add_library(ibex_hydrax SHARED ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(ibex_hydrax ${Boost_LIBRARIES} ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_LIBS})

install(TARGETS ibex_hydrax
     RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/" COMPONENT libraries
     LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/" COMPONENT libraries
     ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/" COMPONENT libraries)
