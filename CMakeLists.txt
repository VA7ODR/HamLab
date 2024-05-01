cmake_minimum_required(VERSION 3.27)

project(HamLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(SOURCE_DIR="${CMAKE_SOURCE_DIR}")

option(ADDRESSSANITIZER "Use AddressSanitizer" OFF)

if (ADDRESSSANITIZER)
         set(ADDRESSSANITIZERFLAGS " -fsanitize=address -fno-omit-frame-pointer ")
endif(ADDRESSSANITIZER)

set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS_INIT} ${ADDRESSSANITIZERFLAGS} -fPIC -rdynamic ")

find_package(SDL2 REQUIRED)
find_package(Boost REQUIRED filesystem system url)
find_package(OpenGL REQUIRED)
find_package(OpenSSL REQUIRED)

set (SOURCE_DIR_DEFINITION SOURCE_DIR="${PROJECT_SOURCE_DIR}")

add_compile_definitions(APP_NAME="${PROJECT_NAME}" APP_VERSION_STRING="1.0.0" ${SOURCE_DIR_DEFINITION} SUPPORT_ORDERED_JSON)

add_subdirectory(EasyAppBase)
include_directories(EasyAppBase
        EasyAppBase/shared_recursive_mutex
        EasyAppBase/imgui
        EasyAppBase/imgui/backends
        EasyAppBase/json_document
        EasyAppBase/json_document/ArbitraryOrderMap
        EasyAppBase/json_document/SDString
        ${SDL2_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

add_subdirectory(wsjtx)
add_subdirectory(logger)
add_subdirectory(rigctl)

add_executable(${PROJECT_NAME} main.cpp HamLabPlugin.hpp HamLabPlugin.cpp PluginLoader.cpp PluginLoader.hpp)
target_link_libraries(${PROJECT_NAME} PRIVATE easy_app_base)

#add_executable(HamLab main.cpp utils.hpp app_logger.cpp app_logger.hpp network.cpp network.cpp thread.cpp thread.hpp eventhandler.cpp eventhandler.hpp HamLabPlugin.hpp PluginLoader.hpp)
#if(TARGET SDL2::SDL2main)
#    target_link_libraries(HamLab PRIVATE SDL2::SDL2main)
#endif()
#target_link_libraries(HamLab PRIVATE json_document imgui utils logger crypto ssl SDL2::SDL2 OpenGL::GL Boost::filesystem Boost::system Boost::url)

