cmake_minimum_required(VERSION 3.28.1)
project(Engine)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

set(PLAT_SRC "platform/src")
set(VK_SRC "${PLAT_SRC}/vulkan")
set(WINAPI_SRC "${PLAT_SRC}/winapi")

set(VULKAN_SOURCES "\
${VK_SRC}/vk_descriptors.cpp;\
${VK_SRC}/vk_engine.cpp;\
${VK_SRC}/vk_images.cpp;\
${VK_SRC}/vk_initialisers.cpp;\
${VK_SRC}/vk_loader.cpp;\
${VK_SRC}/vk_pipelines.cpp;\
")

set(WINAPI_SOURCES "\
${WINAPI_SRC}/data_type.cpp;\
${WINAPI_SRC}/game_window.cpp;\
${WINAPI_SRC}/input.cpp;\
")

set(GAME_SOURCES "\
game/src/game_logic.cpp;\
")

set(MAIN_SOURCES "\
src/game_engine.cpp;\
")

add_library(Engine STATIC)
target_sources(Engine PRIVATE ${VULKAN_SOURCES} ${WINAPI_SOURCES} ${GAME_SOURCES} ${MAIN_SOURCES})
target_link_libraries(Engine PUBLIC vma glm Vulkan::Vulkan stb_image vkbootstrap imgui dwmapi)
target_link_libraries(Engine INTERFACE glm)
target_include_directories(Engine PUBLIC "platform/include")
target_include_directories(Engine PUBLIC "game/include")
target_include_directories(Engine PUBLIC "include")
target_compile_definitions(Engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)