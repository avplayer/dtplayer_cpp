cmake_minimum_required(VERSION 2.8)
project(dtplayer)

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG(-std=c++0x LIBCXX_HAS_STDCXX0X_FLAG)

if(LIBCXX_HAS_STDCXX0X_FLAG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_STATIC_RUNTIME    ON)

find_package( ffmpeg )
find_package( SDL2  REQUIRED)
find_package( faad )
find_package( faac )
find_package( Threads )

add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D__STDC_LIMIT_MACROS)

#dont't define this, libbt need depreated api
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${FFMPEG_INCLUDE_DIR})
include_directories(${SDL2_INCLUDE_DIR})

include_directories(dtutils)
include_directories(dtplayer)
include_directories(dthost)
include_directories(dtport)
include_directories(dtdemux)
include_directories(dtaudio)
include_directories(dtvideo)

add_subdirectory(dtutils)
add_subdirectory(dtplayer)
add_subdirectory(dthost)
add_subdirectory(dtport)
add_subdirectory(dtdemux)
add_subdirectory(dtstream)
add_subdirectory(dtvideo)
add_subdirectory(dtaudio)


add_executable(dtmplayer tools/dt_player.cpp tools/version.cpp tools/ao_sdl2.cpp tools/vo_sdl2.cpp tools/ui.cpp tools/render.cpp)
target_link_libraries(dtmplayer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(dtmplayer dtplayercore  dthost dtport dtdemux dtstream dtvideo dtaudio dtutils)


