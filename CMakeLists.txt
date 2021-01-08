cmake_minimum_required(VERSION 3.1)

set(SHORT_NAME cmaketest)
project(${SHORT_NAME} C)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_C_STANDARD 99)

set(MACOSX_FILES "")
if(APPLE)
    set(MACOSX_FILES
        res/julius.icns
        res/sounds/click.wav
        res/sounds/click.mp3
    )
endif()

set(SOURCE_FILES
    src/main.c
    ${MACOSX_FILES}
)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_executable(${SHORT_NAME} WIN32 ${SOURCE_FILES})

if(APPLE)
    # generating a macOS icns file (see https://stackoverflow.com/a/20703594)
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/res/julius.icns
        COMMAND mkdir -p julius.iconset
        COMMAND sips -z 16 16    julius_256.png --out julius.iconset/icon_16x16.png
        COMMAND sips -z 32 32    julius_256.png --out julius.iconset/icon_16x16@2x.png
        COMMAND sips -z 32 32    julius_256.png --out julius.iconset/icon_32x32.png
        COMMAND sips -z 64 64    julius_256.png --out julius.iconset/icon_32x32@2x.png
        COMMAND sips -z 128 128  julius_256.png --out julius.iconset/icon_128x128.png
        COMMAND sips -z 256 256  julius_256.png --out julius.iconset/icon_128x128@2x.png
        COMMAND sips -z 256 256  julius_256.png --out julius.iconset/icon_256x256.png
        COMMAND sips -z 512 512  julius_512.png --out julius.iconset/icon_256x256@2x.png
        COMMAND iconutil -c icns julius.iconset
        COMMAND rm -R julius.iconset
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/res)
    set_source_files_properties(res/julius.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    set_source_files_properties(
        res/sounds/click.wav
        res/sounds/click.mp3
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )

    # setting variables that will populate Info.plist
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.bvschaik.cmaketest")
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(MACOSX_BUNDLE_ICON_FILE "julius.icns")
    set(MACOSX_BUNDLE_BUNDLE_VERSION
        "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${VERSION_REVISION}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MACOSX_BUNDLE_BUNDLE_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MACOSX_BUNDLE_BUNDLE_VERSION})

    set_target_properties(${SHORT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/res/Info.plist.in")

    set(DIRS "")
    set(LIBS "")

    # if SDL2 library is a framework, we need to indicate to CMake
    # the path to its dependencies. SDL2_LIBRARY contains two parts.
    # Example: /Library/Frameworks/SDL2.framework;-framework Cocoa
    # So strip out everything after the ';'
    string(REGEX REPLACE ";.*$" "" SDL2_LIB_DIR "${SDL2_LIBRARY}")
    if(EXISTS "${SDL2_LIB_DIR}/Versions/A/Frameworks")
        list(APPEND DIRS "${SDL2_LIB_DIR}/Versions/A/Frameworks")
    endif()

    # if SDL2_mixer library is a framework, we need to indicate to CMake
    # the path to its dependencies (Ogg.framework etc):
    if(EXISTS "${SDL2_MIXER_LIBRARY}/Versions/A/Frameworks")
        list(APPEND DIRS "${SDL2_MIXER_LIBRARY}/Versions/A/Frameworks")
    endif()

    # when installing, "fixup" automatically copies libraries inside the
    # bundle and links the binary against them
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app \"${LIBS}\" \"${DIRS}\")
    " BUNDLE DESTINATION ${CMAKE_BINARY_DIR})
endif()

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2_MIXER_INCLUDE_DIR})

include_directories(ext)
include_directories(src)
if(MSVC)
    include_directories(ext/dirent)
endif()

if (UNIX AND NOT APPLE AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang"))
    target_link_libraries(${SHORT_NAME} m)
endif()
target_link_libraries(${SHORT_NAME} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})
if(NOT APPLE AND NOT ANDROID_BUILD)
    install(TARGETS ${SHORT_NAME} RUNTIME DESTINATION bin)
endif()
