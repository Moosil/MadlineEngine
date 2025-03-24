cmake_minimum_required (VERSION 3.28.1)

set(CMAKE_CXX_STANDARD 23)

project(MadlineEngine)

add_subdirectory(third_party)

add_subdirectory(Engine)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Engine PRIVATE "BUILD_TYPE=0")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Engine PRIVATE "BUILD_TYPE=1")
else()
    message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}. Only 'Debug' and 'Release' are allowed.")
endif()

add_subdirectory(CelestePet)