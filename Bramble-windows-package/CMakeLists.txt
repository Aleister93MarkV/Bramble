cmake_minimum_required(VERSION 3.16)
project(CrystalAudioPlayer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /utf-8")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native -ffast-math")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)
find_package(PkgConfig REQUIRED)

if(WIN32)
    set(SDL2_LIBRARY "SDL2")
    set(LIBSNDFILE_LIBRARY "sndfile")
    set(OPENMPT_LIBRARY "openmpt")
else()
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(LIBSNDFILE REQUIRED sndfile)
    pkg_check_modules(OPENMPT REQUIRED libopenmpt)
endif()

set(SOURCES
    src_cpp/main.cpp
    src_cpp/MainWindow.cpp
    src_cpp/AudioEngine.cpp
    src_cpp/EqualizerWindow.cpp
    src_cpp/VisualizerWindow.cpp
    src_cpp/SkinWindow.cpp
    src_cpp/foobar2k_sdk/dsp_manager.cpp
    src_cpp/foobar2k_sdk/eq_dsp.cpp
    src_cpp/foobar2k_sdk/crystallizer_dsp.cpp
)

set(HEADERS
    src_cpp/MainWindow.hpp
    src_cpp/AudioEngine.hpp
    src_cpp/EqualizerWindow.hpp
    src_cpp/VisualizerWindow.hpp
    src_cpp/SkinWindow.hpp
    src_cpp/SkinManager.hpp
    src_cpp/foobar2k_sdk/service_base.hpp
    src_cpp/foobar2k_sdk/dsp.hpp
    src_cpp/foobar2k_sdk/dsp_manager.hpp
    src_cpp/foobar2k_sdk/eq_dsp.hpp
    src_cpp/foobar2k_sdk/crystallizer_dsp.hpp
)

set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src_cpp
)

if(WIN32)
    list(APPEND INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
endif()

include_directories(${INCLUDE_DIRS})

if(WIN32)
    set(PLATFORM_LIBS Qt6::Widgets Qt6::Gui Qt6::Core winmm libsndfile-1 libopenmpt)
    if(MSVC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:wmainCRTStartup")
    endif()
else()
    set(PLATFORM_LIBS Qt6::Widgets Qt6::Gui Qt6::Core ${LIBSNDFILE_LIBRARIES} ${OPENMPT_LIBRARIES} pthread)
endif()

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${PLATFORM_LIBS})

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "CrystalAudioPlayer")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "CrystalAudioPlayer")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

message(STATUS "Building CrystalAudioPlayer")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
