cmake_minimum_required (VERSION 3.28.1)

set(CMAKE_CXX_STANDARD 23)

project(MadlineEngine)

add_subdirectory(third_party)

add_subdirectory(Engine)

target_compile_definitions(Engine PRIVATE "BUILD_TYPE=${CMAKE_BUILD_TYPE}")

add_subdirectory(CelestePet)