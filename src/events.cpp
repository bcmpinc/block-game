/*
    Voxel-Engine - A CPU based sparse octree renderer.
    Copyright (C) 2013  B.J. Conijn <bcmpinc@users.sourceforge.net>

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

#define GLM_FORCE_RADIANS 
#include <glm/gtc/matrix_transform.hpp>
#include <SDL/SDL.h>

#include "events.h"

// Buttons
class button {
    public:
    enum {
        W, A, S, D, 
        SPACE,
        
        STATES
    };
};

static const double ROTATE_SPEED =  0.01;
static const double MOVE_SPEED = 0.15;
static const double JUMP_SPEED = 0.4;
static const int MILLISECONDS_PER_FRAME = 33;
static const double GROUND_CONTROL = 0.3;
static const double AIR_CONTROL = 0.05;
static const glm::dvec3 GRAVITY(0,-0.02,0);

static bool button_state[button::STATES];
static bool mousemove=false;

bool airborne = false;
bool quit  = false;
bool moves = true;
glm::dmat3 orientation;
glm::dvec3 position;
glm::dvec3 velocity;
double tau=0, phi=0;

// checks user input
void handle_events() {
    moves=false;

    SDL_Event event;
  
    /* Check for events */
    while (SDL_PollEvent (&event)) {
        switch (event.type) {
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            bool state = (event.type == SDL_KEYDOWN);
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_w:
                    button_state[button::W] = state;
                    break;
                case SDLK_a:
                    button_state[button::A] = state;
                    break;
                case SDLK_s:
                    button_state[button::S] = state;
                    break;
                case SDLK_d:
                    button_state[button::D] = state;
                    break;
                case SDLK_SPACE:
                    button_state[button::SPACE] = state;
                    break;
                default:
                    break;
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            SDL_ShowCursor(mousemove);
            mousemove=!mousemove;
            SDL_WM_GrabInput(mousemove?SDL_GRAB_ON:SDL_GRAB_OFF);
            break;
        }
        case SDL_MOUSEMOTION: {
            if (mousemove) {
                tau -= event.motion.xrel*ROTATE_SPEED;
                phi -= event.motion.yrel*ROTATE_SPEED;
                if (tau> M_PI) tau -= 2*M_PI;
                if (tau<-M_PI) tau += 2*M_PI;
                if (phi> M_PI/2) phi =  M_PI/2;
                if (phi<-M_PI/2) phi = -M_PI/2;
                moves=true;
            }
            break;
        }
        case SDL_QUIT:
            quit = true;
            break;
        default:
            break;
        }
    }
    
    glm::dmat4 view;
    view = glm::rotate(view, tau, glm::dvec3(0,1,0));
    
    glm::dmat3 M = glm::transpose(glm::dmat3(view));
    
    
    // Apply drag.
    if (airborne) {
        velocity *= (1-AIR_CONTROL);
    } else {
        velocity *= (1-GROUND_CONTROL);
    }
    
    // Apply control
    double dist = (airborne?AIR_CONTROL:GROUND_CONTROL) * MOVE_SPEED;
    if (button_state[button::W]) {
        velocity += dist * M[2];
    }
    if (button_state[button::S]) {
        velocity -= dist * M[2];
    }
    if (button_state[button::A]) {
        velocity -= dist * M[0];
    }
    if (button_state[button::D]) {
        velocity += dist * M[0];
    }
    if (airborne) {
        // Fall
        velocity += GRAVITY;
    } else {
        // Jump
        if (button_state[button::SPACE]) {
            velocity += JUMP_SPEED * M[1];
            airborne = true;
        }
    }
    
    // Move
    if (glm::length(velocity)>1e-3) {
        position += velocity;
        moves = true;
    }
    
    view = glm::rotate(view, phi, M[0]);
    
    orientation = glm::dmat3(view);
} 

void next_frame(int elapsed) {
    int delay = MILLISECONDS_PER_FRAME-elapsed;
    if (delay>10) {
        SDL_Delay(delay);
    }    
}
