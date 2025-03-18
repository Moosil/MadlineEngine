cmake_minimum_required(VERSION 3.28.1)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

file(RELATIVE_PATH GAME_PROJECT_NAME ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_LIST_DIR})

project(GAME_PROJECT_NAME)

add_executable(${GAME_PROJECT_NAME} main.cpp;)

target_link_libraries(${GAME_PROJECT_NAME} Engine)
target_include_directories(${GAME_PROJECT_NAME} PUBLIC include)

add_subdirectory(shaders)