cmake_minimum_required(VERSION 3.28)
project(CelestePet)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE VULKAN_SOURCES CONFIGURE_DEPENDS "platform/src/vulkan/*.cpp")
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "platform/src/windows/*.cpp")
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "game/src/*.cpp")
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(CelestePet main.cpp; ${VULKAN_SOURCES} ${ENGINE_SOURCES} ${GAME_SOURCES} ${MAIN_SOURCES})
target_link_libraries(CelestePet PRIVATE Vulkan::Vulkan)
target_include_directories(CelestePet PUBLIC "thirdparty/")
target_include_directories(CelestePet PUBLIC "platform/include/")
target_include_directories(CelestePet PUBLIC "game/include/")
target_include_directories(CelestePet PUBLIC "include/")