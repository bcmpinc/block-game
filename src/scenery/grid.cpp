#include <GL/gl.h>

#include "../point_types.h"
#include "../events.h"
#include "scenery.h"

static const int GRID_SIZE = 1023;
static point3s grid_array[GRID_SIZE * 4];

struct grid;

template<>
void scenery<grid>::init(lua_State*) {
    for (int i=0; i<GRID_SIZE; i++) {
        grid_array[4*i+0]={-GRID_SIZE/2,0,(short)(i-GRID_SIZE/2)};
        grid_array[4*i+1]={+GRID_SIZE/2,0,(short)(i-GRID_SIZE/2)};
        grid_array[4*i+2]={(short)(i-GRID_SIZE/2),0,-GRID_SIZE/2};
        grid_array[4*i+3]={(short)(i-GRID_SIZE/2),0, GRID_SIZE/2};
    }
}

template<>
void scenery<grid>::clear() {
}

template<>
void scenery<grid>::draw() {
    grid_array->attach();
    glColor4f(0,1,0, 0.3);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1);
    glDrawArrays(GL_LINES, 0, GRID_SIZE*4);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

template<>
void scenery<grid>::interact() {
    // Ground collision.
    if (position.y <= PLAYER_SIZE + COLLISION_EPSILON) {
        position.y = PLAYER_SIZE;
        velocity.y = 0;
        airborne = false;
    }
}
