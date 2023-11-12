cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(LangtonsAnt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(ext/GraphicsBase)
file(GLOB SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} gfxbase)
