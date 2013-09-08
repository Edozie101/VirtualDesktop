project(ibex)

cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
SET(CMAKE_MODULE_PATH "/usr/share/OGRE/cmake/modules" ${CMAKE_MODULE_PATH})

if(!OGRE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D=_USE_SIXENSE=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_SIXENSE=1")
    set(SIXENSE_LIBS sixense_x64 sixense_utils_x64)
endif(!OGRE)

find_package(OpenGL REQUIRED)
find_package(OpenCV)
find_package(VLC)
find_package(OculusSDK)

include_directories( ${OCULUS_INCLUDE_DIRS} ${VLC_INCLUDE_DIRS} )

IF(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64") #i386;x86_64")

    SET (CMAKE_C_COMPILER             "/usr/bin/clang")
    SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")

   SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "Deployment target for OSX" FORCE)

   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(APPLICATIONSERVICES_LIBRARY ApplicationServices)
   FIND_LIBRARY(VIDEODECODEACCELERATION_LIBRARY VideoDecodeAcceleration)
   FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo)
   FIND_LIBRARY(IOKIT_LIBRARY IOKit)
   FIND_LIBRARY(OPENAL_LIBRARY OpenAL)
   FIND_LIBRARY(APPKIT_LIBRARY AppKit)
   FIND_LIBRARY(QUARTZ_CORE_LIBRARY QuartzCore)
   FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
   FIND_LIBRARY(CARBON_LIBRARY Carbon)
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
   FIND_LIBRARY(AGL_LIBRARY AGL )

   MARK_AS_ADVANCED (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY
                     APPLICATIONSERVICES_LIBRARY
                     VIDEODECODEACCELERATION_LIBRARY
                     COREVIDEO_LIBRARY
                     IOKIT_LIBRARY
                     OPENAL_LIBRARY
                     QUARTZ_CORE_LIBRARY
                     COREFOUNDATION_LIBRARY
                     CARBON_LIBRARY
                     AGL_LIBRARY
                     APPKIT_LIBRARY)

   SET(EXTRA_LIBS ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY} 
                    ${APPLICATIONSERVICES_LIBRARY}
                    ${VIDEODECODEACCELERATION_LIBRARY}
                    ${COREVIDEO_LIBRARY}
                    ${IOKIT_LIBRARY}
                    ${OPENAL_LIBRARY}
                    ${QUARTZ_CORE_LIBRARY}
                    ${COREFOUNDATION_LIBRARY}
                    ${CARBON_LIBRARY}
                    ${APPKIT_LIBRARY}
                    ${AGL_LIBRARY}
          #boost_context-mt boost_system-mt boost_filesystem-mt
          #boost_thread-mt boost_atomic-mt boost_chrono-mt boost_date_time-mt
   )
   
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBOOST_ALL_NO_LIB -fvisibility-inlines-hidden -Wno-sign-conversion -arch x86_64 -Wall -g -O0 -ggdb -fobjc-arc -DHAVE_LIBJPEG=1 ${OCULUS_INCLUDE_FLAGS} ${VLC_INCLUDE_FLAGS}  -mmacosx-version-min=10.8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_ALL_NO_LIB -fvisibility-inlines-hidden -Wno-sign-conversion -arch x86_64 -std=c++11 -stdlib=libc++ -Wall -frtti -fexceptions -g  -O0 -ggdb -fobjc-arc -DHAVE_LIBJPEG=1 ${OCULUS_INCLUDE_FLAGS} ${VLC_INCLUDE_FLAGS}  -mmacosx-version-min=10.8")
    set(ibex PROPERTIES LINK_FLAGS  -fobjc-link-runtime -mmacosx-version-min=10.8)
ENDIF (APPLE)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Linux specific code
    SET(OperatingSystem "Linux")

    find_package(GLUT REQUIRED)
    find_package(OpenAL)
    find_package(Irrlicht)
    find_package(OGRE)
    find_package(OIS)

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_RUNTIME OFF)
    find_package(Boost 1.54.0 COMPONENTS filesystem system thread atomic chrono date_time context)

    if( IRRLICHT_FOUND AND IRRLICHT )  
      add_definitions( -DENABLE_IRRLICHT )
      set(IRRLICHT_INCLUDE_FLAGS "-I${IRRLICHT_INCLUDE_DIRS}")
      set(IRRLICHT_PLUGIN_FILES "irrlicht_plugin/irrlicht_plugin.cpp")
    endif( IRRLICHT_FOUND AND IRRLICHT )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IRRLICHT_INCLUDE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IRRLICHT_INCLUDE_FLAGS}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


if(OGRE)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_RUNTIME OFF)
    find_package(boost 1.54.0 COMPONENTS filesystem system thread atomic chrono date_time context)

    find_package(OGRE)
    #find_package(OIS)

    if( OGRE_FOUND  )
      add_definitions( -DENABLE_OGRE3D )
      #set(OGRE_INCLUDE_FLAGS "-I${OGRE_INCLUDE_DIRS}")
      set(OGRE_PLUGIN_FILES "ogre3d_plugin/BaseApplication.cpp" "ogre3d_plugin/TutorialApplication.cpp" "ogre3d_plugin/DotSceneLoader.cpp")

      include_directories( ${OIS_INCLUDE_DIRS}
            ${OGRE_INCLUDE_DIRS}
            ${OGRE_SAMPLES_INCLUDEPATH}
    )
    endif( OGRE_FOUND )

    IF(APPLE)
        set(SRC
            ${SRC}
            MyOgreView.mm
        )
    endif(APPLE)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OGRE_INCLUDE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OGRE_INCLUDE_FLAGS}")
endif(OGRE)

add_subdirectory(windows)
add_subdirectory(filesystem)
add_subdirectory(video)
add_subdirectory(sixense)
add_subdirectory(oculus)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX_SOURCES opengl_setup_x11.cpp opengl_setup_x11.h)

    add_subdirectory(glm)
    add_subdirectory(x11)
    set(LINUX_LIBS 
            ibex_x11 ibex_glm
            GL GLU glut GLEW
            openal
            X11 Xfixes Xdamage Xrandr Xrender Xcomposite Xxf86vm Xi Xext Xinerama udev pthread
            jpeg rt
    )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${Boost_INCLUDE_DIR}
 )
link_directories(${Boost_LIBRARY_DIR} ${OCULUS_LIBRARY_DIR} ${VLC_LIBRARY_DIR} /usr/local/lib ${OGRE_LIBS_DIR})

set(RESOURCES README.md COPYING AUTHORS)

IF(APPLE)
    include (InstallRequiredSystemLibraries)

    set(RESOURCES MainWindow.xib)

    set(OSX_SOURCES
        main.mm
        IbexMacUtils.mm
        AppDelegate.mm
        MyOpenglView.mm
        MyWindow.mm
        ScreenshotView.mm
        ServerController.mm
    )
    set(OSX_LIBS c++)
    set(VLC_LIBRARIES "${VLC_LIBRARY_DIR}/libvlc.5.dylib" "${VLC_LIBRARY_DIR}/libvlccore.5.dylib")
ENDIF(APPLE)

set(SRC ${SRC}
    RendererPlugin.cpp RendererPlugin.h 
    distortions.cpp distortions.h 
    ibex.cpp ibex.h 
    opengl_helpers.cpp opengl_helpers.h
    GL/glew.c
    utils.cpp utils.h
    simpleworld_plugin/SimpleWorldRendererPlugin.cpp simpleworld_plugin/SimpleWorldRendererPlugin.h
    iphone_orientation_plugin/iphone_orientation_listener.cpp iphone_orientation_plugin/iphone_orientation_listener.h
    tinyxml/tinystr.cpp
    tinyxml/tinyxml.cpp
    tinyxml/tinyxmlerror.cpp
    tinyxml/tinyxmlparser.cpp
    ${OSX_SOURCES}
    ${LINUX_SOURCES}
)

add_executable(ibex MACOSX_BUNDLE WIN32 ${SRC} ${IRRLICHT_PLUGIN_FILES} ${OGRE_PLUGIN_FILES})
target_link_libraries(ibex 
    ibex_windows ibex_filesystem
    ibex_video
    ibex_sixense
    ${SIXENSE_LIBS}
    ${LINUX_LIBS}
     ibex_oculus
    opencv_core opencv_highgui opencv_video opencv_contrib opencv_ts opencv_legacy
    ovr
    ${VLC_LIBRARIES}
    #${Boost_LIBRARIES} boost_date_time-mt boost_exception-mt
    ${EXTRA_LIBS}
    ${IRRLICHT_LIBRARY} ${OGRE_LIBRARIES} ${OIS_LIBRARIES}
    ${OGRE_LIBS}
    ${OSX_LIBS}
)
 
set_target_properties( ibex PROPERTIES RESOURCE "${RESOURCES}" )

#--------------------------------------------------------------------------------
# For Apple set the icns file containing icons
IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE ibex)

  # set where in the bundle to put the icns file
#  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/ibex.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
#  SET(SRC ${SRC} ibex.iconset)

# These variables are specific to our plist and are NOT standard CMake variables
set_target_properties(ibex PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
set_target_properties(ibex PROPERTIES MACOSX_BUNDLE_NSMAIN_NIB_FILE "MainMenu")
set_target_properties(ibex PROPERTIES MACOSX_BUNDLE_NSPRINCIPAL_CLASS "NSApplication")

ENDIF(APPLE)


#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
SET(plugin_dest_dir bin)
SET(APPS "ibex")
IF(APPLE)
  SET(plugin_dest_dir ibex.app/Contents/MacOS)
  SET(APPS "${CMAKE_INSTALL_PREFIX}/ibex.app")
  
  install (DIRECTORY resources DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Resources/")
  install (DIRECTORY ${VLC_LIBRARY_DIR}/../plugins DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/")
  #install (DIRECTORY ${VLC_LIBRARY_DIR} DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/")

file(GLOB VLC_FILES "${VLC_LIBRARY_DIR}/*")
file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/MacOS/"
   TYPE FILE FILES ${VLC_FILES})

ENDIF(APPLE)

#--------------------------------------------------------------------------------
# Install the ibex application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS ibex
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )
    
INSTALL(CODE "
    execute_process(COMMAND \"iconutil\" --convert icns --output \"${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Resources/ibex.icns\" \"${PROJECT_SOURCE_DIR}/ibex.iconset\")
    execute_process(COMMAND rm \"${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Resources/ibex.iconset\")
")

install (DIRECTORY resources DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

IF(APPLE)
    set(BU_COPY_FULL_FRAMEWORK_CONTENTS ON)
    # Now the work of copying dependencies into the bundle/package
    # The quotes are escaped and variables to use at install time have their $ escaped
    # An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
    # Note that the image plugins depend on QtSvg and QtXml, and it got those copied
    # over.
    if(OGRE)
    INSTALL(CODE "
        execute_process(COMMAND \"rm -rf ${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks/*.framework\")
        " COMPONENT Runtime)
    endif(OGRE)

    INSTALL(CODE "
      # If BU_CHMOD_BUNDLE_ITEMS is not set install_name_tool will fail to update some dylibs due to them being read-only
      SET(BU_CHMOD_BUNDLE_ITEMS ON)
      execute_process(COMMAND \"rm -rf ${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks\")
      include(BundleUtilities)
        execute_process(COMMAND rm -rf \"${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks\")
        file(GLOB_RECURSE IBEX_LIBS \"${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
          execute_process(COMMAND \"/usr/bin/rm -rf ${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks\")
        fixup_bundle(\"${APPS}\" \"${IBEX_LIBS}\" \"${DIRS}\")
      execute_process(COMMAND /usr/bin/rm \"-rf ${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks\")
        " COMPONENT Runtime)
        execute_process(COMMAND /usr/bin/mkdir \"-p ${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks/\")
    execute_process(COMMAND /usr/bin/cp -fR \"~/Downloads/OgreSDK/lib/macosx/Release/*\" \"${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks/\")
    #INSTALL(CODE "
    #    file(GLOB_RECURSE IBEX_LIBS \"\${CMAKE_INSTALL_PREFIX}/*\")
     #   include(BundleUtilities)
       # fixup_bundle(\"${APPS}\" \"\" \"\")
      #  " COMPONENT Runtime)

    #install_name_tool -change "@loader_path/../lib/libvlccore.5.dylib" "@loader_path/../MacOS/libvlccore.5.dylib"   liblibbluray_plugin.dylib
    INSTALL(CODE "
        file(GLOB VLC_PLUGINS RELATIVE \"${CMAKE_INSTALL_PREFIX}\" \"${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/plugins/*.dylib\")
        foreach(f \${VLC_PLUGINS})
            execute_process(COMMAND \"install_name_tool\" -change \"@loader_path/../lib/libvlccore.5.dylib\" \"@loader_path/../MacOS/libvlccore.5.dylib\" ${CMAKE_INSTALL_PREFIX}/\${f})
        endforeach(f)
    " COMPONENT Runtime)



    # these are the OS X Interface Builder Files
    set (ibex_XIBS
      MainMenu
    )

    # Make sure we can find the 'ibtool' program. If we can NOT find it we
    # skip generation of this project
    find_program(IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
    if (${IBTOOL} STREQUAL "IBTOOL-NOTFOUND")
      message(SEND_ERROR "ibtool can not be found and is needed to compile the .xib files. It should have been installed with
                        the Apple developer tools. The default system paths were searched in addition to ${OSX_DEVELOPER_ROOT}/usr/bin")
    endif()

    # Make sure the 'Resources' Directory is correctly created before we build
    add_custom_command (TARGET ibex PRE_BUILD
                          COMMAND mkdir -p "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Resources")

    # Compile the .xib files using the 'ibtool' program with the destination being the app package
    foreach(xib ${ibex_XIBS})
      add_custom_command (TARGET ibex POST_BUILD
                          COMMAND ${IBTOOL} --errors --warnings --notices --output-format human-readable-text
                                  --compile "${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Resources/${xib}.nib"
                                 ${PROJECT_SOURCE_DIR}/en.lproj/${xib}.xib
                          DEPENDS ${xib}
                          COMMENT "Compiling ${PROJECT_SOURCE_DIR}/en.lproj/${xib}.xib")
    endforeach()
    foreach(xib ${ibex_XIBS})
      INSTALL(CODE "
          execute_process(COMMAND \"${IBTOOL} --errors --warnings --notices --output-format human-readable-text
                                  --compile ${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Resources/${xib}.nib
                                 ${PROJECT_SOURCE_DIR}/en.lproj/${xib}.xib\")
                                 " COMPONENT Runtime)
    endforeach()

    #if(OGRE)
    INSTALL(CODE "
        execute_process(COMMAND bash -c \"cp -fR ~/Downloads/OgreSDK/lib/macosx/Release/* ${CMAKE_INSTALL_PREFIX}/ibex.app/Contents/Frameworks/\")
        " COMPONENT Runtime)
    #endif(OGRE)

    # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
    # where CPackConfig.cmake is created by including CPack
    # And then there's ways to customize this as well
    #set(CPACK_BINARY_DRAGNDROP ON)
    #include(CPack)
ENDIF(APPLE)
