cmake_minimum_required (VERSION 2.6)
project(Blockgame)

# REQUIRED does not work in CMake <=2.4.6 for SDL
find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)
find_package(PhysFS REQUIRED)

include_directories(${SDL_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${PHYSFS_INCLUDE_DIR})

SET(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -Wextra")

add_executable(blockgame 
    src/main.cpp
    src/art_gl.cpp
    src/events.cpp
    src/timing.cpp
    src/scene.cpp
    src/point_types.cpp
    src/luaX.cpp
    src/scenery/grid.cpp
    src/scenery/block.cpp
    src/scenery/gems.cpp
    src/scenery/fade.cpp
) 
target_link_libraries(blockgame 
    ${SDL_LIBRARY} 
    ${OPENGL_LIBRARY} 
    ${LUA_LIBRARY}
    ${PHYSFS_LIBRARY}
)

add_executable(map_convert 
    src/map_convert/map_convert.cpp
)
add_definitions("-DGLM_FORCE_RADIANS")
