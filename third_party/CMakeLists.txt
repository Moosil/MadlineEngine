cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)

project(CelestePet)

find_package(Vulkan REQUIRED)

add_library(vkbootstrap STATIC)
add_library(glm INTERFACE)
add_library(vma INTERFACE)

add_library(stb_image INTERFACE)

target_sources(vkbootstrap PRIVATE
        vkbootstrap/VkBootstrap.h
        vkbootstrap/VkBootstrap.cpp
)

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)
set_property(TARGET vkbootstrap PROPERTY CXX_STANDARD 20)

#both vma and glm and header only libs so we only need the include path
target_include_directories(vma INTERFACE vma)

target_include_directories(glm INTERFACE glm)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE
        imgui/imgui.h
        imgui/imgui.cpp

        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_impl_vulkan.cpp
        imgui/imgui_impl_win32.cpp
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)

target_include_directories(stb_image INTERFACE stb_image)
