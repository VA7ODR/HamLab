cmake_minimum_required(VERSION 3.5)

project(HamLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ADDRESSSANITIZER "Use AddressSanitizer" OFF)

if (ADDRESSSANITIZER)
         set(ADDRESSSANITIZERFLAGS " -fsanitize=address -fno-omit-frame-pointer ")
endif(ADDRESSSANITIZER)

set(CMAKE_CXX_FLAGS                " ${CMAKE_CXX_FLAGS_INIT} ${ADDRESSSANITIZERFLAGS} -fPIC -rdynamic ")

add_subdirectory(json)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(Boost REQUIRED)

include_directories(json imgui imgui/backends ${SDL2_INCLUDE_DIRS} magic_enum/include)

add_library(imgui STATIC imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_sdl2.cpp imgui/backends/imgui_impl_sdlrenderer2.cpp)

add_executable(HamLab main.cpp utils.cpp utils.hpp wsjtxreceiver.cpp wsjtxreceiver.hpp)
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(HamLab PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(HamLab PRIVATE json_document imgui SDL2::SDL2)
