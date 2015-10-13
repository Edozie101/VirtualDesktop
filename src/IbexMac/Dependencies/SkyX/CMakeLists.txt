set(HEADER_FILES
	AtmosphereManager.h
	BasicController.h
	CloudsManager.h
	ColorGradient.h
	Controller.h
	GPUManager.h
	MeshManager.h
	MoonManager.h
	Prerequisites.h
	SkyX.h
	VCloudsManager.h
	VClouds/DataManager.h
	VClouds/Ellipsoid.h
	VClouds/FastFakeRandom.h
	VClouds/GeometryBlock.h
	VClouds/GeometryManager.h
	VClouds/Lightning.h
	VClouds/LightningManager.h
	VClouds/VClouds.h
)

set(SOURCE_FILES
	AtmosphereManager.cpp
	BasicController.cpp
	CloudsManager.cpp
	ColorGradient.cpp
	GPUManager.cpp
	MeshManager.cpp
	MoonManager.cpp
	Prerequisites.cpp
	SkyX.cpp
	VCloudsManager.cpp
	VClouds/DataManager.cpp
	VClouds/Ellipsoid.cpp
	VClouds/FastFakeRandom.cpp
	VClouds/GeometryBlock.cpp
	VClouds/GeometryManager.cpp
	VClouds/Lightning.cpp
	VClouds/LightningManager.cpp
	VClouds/VClouds.cpp
)

include_directories (.)

add_library(ibex_skyx SHARED ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(ibex_skyx ${Boost_LIBRARIES} ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_LIBS})

install(TARGETS ibex_skyx
     RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/" COMPONENT libraries
     LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/" COMPONENT libraries
     ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/" COMPONENT libraries)
