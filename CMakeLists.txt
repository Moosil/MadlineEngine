cmake_minimum_required(VERSION 3.28)
project(CelestePet)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/game/src/*.cpp")
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/src/*.cpp")
file(GLOB_RECURSE VULKAN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/platform/vulkan/src/*.cpp")

add_executable(CelestePet main.cpp; ${ENGINE_SOURCES} ${GAME_SOURCES} ${VULKAN_SOURCES})
target_link_libraries(CelestePet PRIVATE Vulkan::Vulkan)
target_include_directories(CelestePet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/game/include")
target_include_directories(CelestePet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/include")
target_include_directories(CelestePet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/platform/vulkan/include")
target_include_directories(CelestePet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw-3.4.bin.WIN64/include")
target_include_directories(CelestePet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")