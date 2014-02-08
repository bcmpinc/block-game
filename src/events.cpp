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

#include <glm/gtc/matrix_transform.hpp>
#include <SDL/SDL.h>
#include <vector>
#include <physfs.h>

#include "events.h"
#include "point_types.h"
#include "keymap.h"

// Buttons
class button {
    public:
    enum {
        FORWARD, BACKWARD, LEFT, RIGHT, 
        JUMP, REWIND, 
        
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
static const int HISTORY = 1024;

static bool button_state[button::STATES];
static bool mousemove=false;
static std::vector<glm::dvec3> old_position;
static std::vector<glm::dvec3> old_velocity;
static double tau=0, phi=0;

bool airborne = false;
bool quit  = false;
glm::dmat3 orientation;
glm::dvec3 position;
glm::dvec3 velocity;
uint move_counter;


void reset(glm::dvec3 start_position) {
    position = start_position;
    velocity = glm::dvec3(0,0,0);
    airborne = false;
    tau=0; phi=0;
    old_position.clear();
    old_velocity.clear();
    move_counter = 0;
}

/** Finishes the enemy recording and moves it to the specified file. 
 */
void finish(glm::dvec3 end_position, const char * target_file) {
    std::vector<point3f> record;
    for (glm::dvec3 p : old_position) {
        point3f pt = {(float)p.x,(float)p.y,(float)p.z};
        record.push_back(pt);
    }
    for (int i=1; i<=8; i++) {
        glm::dvec3 p = (
            old_position.back()*(double)(8-i) + 
            old_velocity.back()*(double)((8-i) * i) + 
            end_position*(double)i
        ) / 8.;
        point3f pt = {(float)p.x,(float)p.y,(float)p.z};
        record.push_back(pt);
    }
    PHYSFS_File * w = PHYSFS_openWrite(target_file);
    assert(w);
    PHYSFS_write(w, record.data(), sizeof(point3f), record.size());
    PHYSFS_close(w);
}

// checks user input
void handle_events() {
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
                case KEY_FORWARD:
                    button_state[button::FORWARD] = state;
                    break;
                case KEY_BACKWARD:
                    button_state[button::BACKWARD] = state;
                    break;
                case KEY_LEFT:
                    button_state[button::LEFT] = state;
                    break;
                case KEY_RIGHT:
                    button_state[button::RIGHT] = state;
                    break;
                case KEY_JUMP:
                    button_state[button::JUMP] = state;
                    break;
                case KEY_REWIND:
                    button_state[button::REWIND] = state;
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
    
    // Yaw camera
    glm::dmat4 view;
    view = glm::rotate(view, tau*180/M_PI, glm::dvec3(0,1,0));
    
    // Obtain current axes
    glm::dmat3 M = glm::transpose(glm::dmat3(view));
    
    if (button_state[button::REWIND]) {
        if (!old_position.empty()) {
            position = old_position.back(); old_position.pop_back();
            velocity = old_velocity.back(); old_velocity.pop_back();
            move_counter--;
        }
    } else {
        glm::dvec3 prev_velocity = velocity;
        
        // Apply drag.
        if (airborne) {
            velocity *= (1-AIR_CONTROL);
        } else {
            velocity *= (1-GROUND_CONTROL);
        }
        
        // Apply control
        double dist = (airborne?AIR_CONTROL:GROUND_CONTROL) * MOVE_SPEED;
        if (
            (button_state[button::FORWARD] != button_state[button::LEFT]) &&
            (button_state[button::BACKWARD] != button_state[button::RIGHT])
        ) {
            dist *= 0.7071;
        }
            
        if (button_state[button::FORWARD]) {
            velocity += dist * M[2];
        }
        if (button_state[button::BACKWARD]) {
            velocity -= dist * M[2];
        }
        if (button_state[button::LEFT]) {
            velocity -= dist * M[0];
        }
        if (button_state[button::RIGHT]) {
            velocity += dist * M[0];
        }
        
        if (airborne) {
            // Fall
            velocity += GRAVITY;
        } else {
            // Jump
            if (button_state[button::JUMP]) {
                velocity += JUMP_SPEED * M[1];
                airborne = true;
            }
        }
        
        // Move
        if (glm::length(velocity)>1e-3) {
            // Record history
            old_position.push_back(position);
            old_velocity.push_back(prev_velocity);
            move_counter++;

            // Move
            position += velocity;
        }
    }
    
    // Tilt camera
    view = glm::rotate(view, phi*180/M_PI, M[0]);
    orientation = glm::dmat3(view);
} 

void next_frame(int elapsed) {
    int delay = MILLISECONDS_PER_FRAME-elapsed;
    if (delay>10) {
        SDL_Delay(delay);
    }    
}
