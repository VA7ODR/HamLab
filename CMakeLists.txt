cmake_minimum_required(VERSION 3.11)

project(HamLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ADDRESSSANITIZER "Use AddressSanitizer" OFF)

if (ADDRESSSANITIZER)
         set(ADDRESSSANITIZERFLAGS " -fsanitize=address -fno-omit-frame-pointer ")
endif(ADDRESSSANITIZER)

set(CMAKE_CXX_FLAGS                " ${CMAKE_CXX_FLAGS_INIT} ${ADDRESSSANITIZERFLAGS} -fPIC -rdynamic ")

add_subdirectory(json)
add_subdirectory(shared_recursive_mutex)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(Boost REQUIRED filesystem system)
find_package(OpenGL REQUIRED)

add_compile_definitions(SUPPORT_ORDERED_JSON)

include_directories(json shared_recursive_mutex imgui imgui/backends ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} magic_enum/include)

add_library(imgui STATIC imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_sdl2.cpp imgui/backends/imgui_impl_opengl3.cpp)

add_library(utils STATIC utils.cpp utils.hpp)

add_subdirectory(wsjtx)
add_subdirectory(logger)
add_subdirectory(rigctl)

add_executable(HamLab main.cpp utils.hpp HamLabPlugin.hpp PluginLoader.hpp)
if(TARGET SDL2::SDL2main)
    target_link_libraries(HamLab PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(HamLab PRIVATE json_document imgui utils logger SDL2::SDL2 OpenGL::GL Boost::filesystem Boost::system)

