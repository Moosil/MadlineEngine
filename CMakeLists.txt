cmake_minimum_required(VERSION 3.28)
project(CelestePet)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party)

file(GLOB_RECURSE VULKAN_SOURCES CONFIGURE_DEPENDS "platform/src/vulkan/*.cpp")
file(GLOB_RECURSE WINDOWS_SOURCES CONFIGURE_DEPENDS "platform/src/windows/*.cpp")
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "game/src/*.cpp")
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(CelestePet ${VULKAN_SOURCES} ${WINDOWS_SOURCES} ${GAME_SOURCES} ${MAIN_SOURCES})
target_link_libraries(CelestePet PRIVATE vma glm Vulkan::Vulkan stb_image vkbootstrap imgui)
target_include_directories(CelestePet PUBLIC "platform/include/")
target_include_directories(CelestePet PUBLIC "game/include/")
target_include_directories(CelestePet PUBLIC "include/")
target_compile_definitions(CelestePet PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)