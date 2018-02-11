block-game
==========

A minimalistic 3D platform game.

The goal is to get to the crystal as fast as possible. The game records how you play, such that the second time you have someone to race against.

Dependencies
------------
This game requires the following libraries.
* GLM
* OpenGL
* SDL
* Lua

Building and running the game
-----------------------------
Execute the following commands:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make
    ./blockgame

When starting the game you can also specify the level you want to start with, for example: `./blockgame 5`.

Note: while you can use a different directory to build and run from, the binary expects to find the maps in `../maps/`.
    
Movement
--------
* wasd-keys: walking around.
* space: jump.
* ctrl: reverse time.
* esc: quit.

If you want to change the keybindings, you have to edit `keymap.h`. A dvorak layout can be selected by replacing
    
    #define KEYBOARD QWERTY 
    
with 

    #define KEYBOARD DVORAK
