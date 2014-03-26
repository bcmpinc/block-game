#include <GL/gl.h>
#include <lua.hpp>

#include "../point_types.h"
#include "../events.h"
#include "scenery.h"
#include "fade.h"

struct fade;

static const int FADE_DURATION = 60;
static int fade_counter;

static point2s fade_coords[]= {
    {-1,-1},{1,-1},{1,1},{-1,1},  
};

template<>
void scenery<fade>::init(lua_State *) {
    fade_counter = FADE_DURATION;
    fade_state = FADE_STATES::FADE_IN;
}

template<>
void scenery<fade>::clear() {
    
}

template<>
void scenery<fade>::draw() {
    if (fade_counter<=0) return;
    fade_coords->attach();
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glColor4f(0,0,0,fade_counter/(float)FADE_DURATION);
    glDrawArrays(GL_QUADS, 0, 4);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

template<>
void scenery<fade>::interact() {
    switch(fade_state) {
        case FADE_STATES::FADE_OUT:
            if (fade_counter < FADE_DURATION) {
                fade_counter++;
            } else {
                fade_state = FADE_STATES::BLACK;
            }
            break;
        case FADE_STATES::FADE_IN:
            if (fade_counter > 0) {
                fade_counter--;
            } else {
                fade_state = FADE_STATES::NONE;
            }
            break;
        default:
            break;
    }
}

