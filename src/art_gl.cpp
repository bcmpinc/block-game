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

#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <SDL/SDL.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "events.h"
#include "art.h"

static const glm::dvec3 CAMERA_OFFSET(0,0.2,0);

using glm::min;
using glm::max;

namespace {
    // The screen surface
    SDL_Surface *screen = NULL;
    /** The projection matrix.
     * Note that we use a left handed axis system, hence we are initially looking down the positive Z-axis.
     * Up is positive Y and right is positive X.
     */
    const glm::dmat4 frustum_matrix = glm::scale(glm::frustum<double>(frustum::left, frustum::right, frustum::bottom, frustum::top, frustum::near, frustum::far),glm::dvec3(1,1,-1));
}

void init_screen(const char * caption) {
    // Initialize SDL 
    if (SDL_Init (SDL_INIT_VIDEO) < 0) {
        fprintf (stderr, "Couldn't initialize SDL: %s\n", SDL_GetError ());
        exit (2);
    }
    atexit (SDL_Quit);
#if defined _WIN32 || defined _WIN64
    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );
#endif

    // Set 32-bits OpenGL video mode
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4);
    // TODO: include SDL_RESIZABLE flag
    screen = SDL_SetVideoMode (SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_OPENGL | (SCREEN_FULLSCREEN*SDL_FULLSCREEN));
    if (screen == NULL) {
        fprintf (stderr, "Couldn't set video mode: %s\n", SDL_GetError ());
        exit (3);
    }
    SDL_WM_SetCaption (caption, NULL);

    // Check OpenGL properties
    printf("OpenGL loaded\n");
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    
    // Set flags
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_COLOR);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDepthFunc(GL_LEQUAL);

    // Frustum
    glMatrixMode(GL_PROJECTION);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // update context viewport size
    glLoadMatrixd(glm::value_ptr(frustum_matrix));
    glMatrixMode(GL_MODELVIEW);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Other
    clear_screen();
    flip_screen();
}

void flip_screen() {
    SDL_GL_SwapBuffers();
}
void clear_screen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void set_matrix() {
    glm::dmat4 view = glm::translate(glm::dmat4(orientation),-position-CAMERA_OFFSET);
    glLoadMatrixd(glm::value_ptr(view));
}
