block-game
==========
A minimalistic 3D platform game.

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7aa2955514134347a9c5d1b8cc39b505)](https://www.codacy.com/app/bcmpinc/block-game?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=bcmpinc/block-game&amp;utm_campaign=Badge_Grade)
[![Build Status](https://travis-ci.org/bcmpinc/block-game.svg?branch=master)](https://travis-ci.org/bcmpinc/block-game)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)


The goal is to get to the crystal as fast as possible. The game records how you play, such that the second time you have someone to race against.

Dependencies
------------
This game requires the following libraries.
* GLM
* OpenGL
* SDL 1.2
* Lua 5.2 or 5.3
* PhysFS

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
