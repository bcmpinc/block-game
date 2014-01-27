cmake_minimum_required (VERSION 2.6)
project(Blockgame)

# REQUIRED does not work in CMake <=2.4.6 for SDL
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED) # if using SDL_image
find_package(OpenGL)

set(blockgame_src
    src/main.cpp
    src/art_gl.cpp
    src/events.cpp
    src/timing.cpp
    src/scene.cpp
    src/point_types.cpp
)

include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})

add_executable(blockgame ${blockgame_src}) 
target_link_libraries(blockgame 
    ${SDL_LIBRARY} 
    ${SDLIMAGE_LIBRARY} 
    ${OPENGL_LIBRARY} 
)
set_property(TARGET blockgame PROPERTY COMPILE_FLAGS "-Wall -std=gnu++11")


