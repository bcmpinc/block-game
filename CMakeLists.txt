cmake_minimum_required (VERSION 2.6)
project(Blockgame)

# REQUIRED does not work in CMake <=2.4.6 for SDL
find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${SDL_INCLUDE_DIR})

SET(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -Wextra")

add_executable(blockgame 
    src/main.cpp
    src/art_gl.cpp
    src/events.cpp
    src/timing.cpp
    src/scene.cpp
    src/point_types.cpp
) 
target_link_libraries(blockgame 
    ${SDL_LIBRARY} 
    ${OPENGL_LIBRARY} 
)

add_executable(map_convert 
    src/map_convert.cpp
)
