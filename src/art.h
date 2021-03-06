/*
    Block Game - A minimalistic 3D platform game
    Copyright (C) 2013,2014  B.J. Conijn <bcmpinc@users.sourceforge.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ART_H
#define ART_H
#include <stdint.h>
#include <glm/glm.hpp>

#define SCREEN_FULLSCREEN  0

#if SCREEN_FULLSCREEN == 1
# define SCREEN_WIDTH    1920
# define SCREEN_HEIGHT   1080
#else
# define SCREEN_WIDTH    1280
# define SCREEN_HEIGHT    960
#endif

void init_screen(const char * caption);
void clear_screen();
void flip_screen();
void set_matrix();

void draw_box();
void draw_cubemap(uint32_t texture); // OpenGL

uint32_t load_texture(const char* filename); // OpenGL
uint32_t load_cubemap(const char* format); // OpenGL

namespace frustum {
    // Compute frustum parameters.
    // left, right, top and bottom are the bounds of the near plane.
    const double aspect = SCREEN_WIDTH/(double)SCREEN_HEIGHT;
    const double left   = -0.05*aspect;
    const double right  =  0.05*aspect;
    const double top    =  0.05;
    const double bottom = -0.05;
    const double near   =  0.08; 
    const double far    =  512;
}

#endif
