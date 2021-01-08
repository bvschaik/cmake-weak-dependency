cmake_minimum_required(VERSION 3.1)
include(CMakeDependentOption)

set(SHORT_NAME julius)
project(${SHORT_NAME} C)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_C_STANDARD 99)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror=implicit-function-declaration")

    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0)
        set(CMAKE_C_FLAGS "-Werror=incompatible-pointer-types -Werror=int-conversion -Wstrict-prototypes ${CMAKE_C_FLAGS}")
    endif()
endif()

set(MACOSX_FILES "")
if(APPLE)
    set(MACOSX_FILES res/julius.icns)
endif()

set(SOURCE_FILES
    src/main.c
    src/sound_device.c
    ${MACOSX_FILES}
)

function(GET_SDL_EXT_DIR result module)
    if(NOT module STREQUAL "")
        set(module "_${module}")
    endif()
    set(SDL_LOCATION ${PROJECT_SOURCE_DIR}/ext/SDL2)
    file(GLOB children
        RELATIVE ${SDL_LOCATION}
        CONFIGURE_DEPENDS
        ${SDL_LOCATION}/SDL${module}
        ${SDL_LOCATION}/SDL2${module}
        ${SDL_LOCATION}/SDL${module}-*
        ${SDL_LOCATION}/SDL2${module}-*
    )
    foreach(child ${children})
        if(IS_DIRECTORY "${SDL_LOCATION}/${child}")
            set(${result} "${SDL_LOCATION}/${child}" PARENT_SCOPE)
            break()
        endif()
    endforeach()
endfunction()

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
    set_source_files_properties(${PROJECT_SOURCE_DIR}/res/julius.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")

    # setting variables that will populate Info.plist
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.bvschaik.julius")
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
        # Copy mpg123 framework manually because since MacOS 10.15, the
        # dependency is seen as "weak" and not copied automatically
        file(COPY "${SDL2_MIXER_LIBRARY}/Versions/A/Frameworks/mpg123.framework" DESTINATION "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks")
        list(APPEND LIBS "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks/mpg123.framework/mpg123")
    endif()

    # when installing, "fixup" automatically copies libraries inside the
    # bundle and links the binary against them
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app \"${LIBS}\" \"${DIRS}\")
        execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} \"-change\" \"@rpath/mpg123.framework/Versions/A/mpg123\" \"@executable_path/../Frameworks/mpg123.framework/mpg123\" \"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks/SDL2_mixer.framework/Versions/A/SDL2_mixer\")
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
