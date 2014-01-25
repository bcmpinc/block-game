#include <cstdint>
#include <vector>
#include <GL/gl.h>

#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene.h"
#include "events.h"
#include "filemap.h"

struct point3s {
    short x,y,z;
};

struct point3f {
    float x,y,z;
};

struct point3fc {
    float x,y,z;
    uint32_t color;
};

struct block {
    float posz;
    float posx;
    float sizex;
    float sizez;
    float top;
    float bottom;
    float rotation; // this is in degrees.
    uint32_t color;
};

struct block_collision {
    glm::dmat3 rotation;
    glm::dvec3 lb;
    glm::dvec3 ub;
};

static const double PLAYER_SIZE = 0.4; 
static const double PLAYER_HEIGHT = 0.6;
static const int GRID_SIZE = 1023;
static point3s grid[GRID_SIZE * 4];
static point3fc * block_coords;
static unsigned short * block_indices;
static block_collision * block_collisions;
static point3f * collision_nodes;
static unsigned int blocks;

glm::dvec3 cube_coords[] = {
    glm::dvec3(-1,-1,-1),
    glm::dvec3(-1,-1, 1),
    glm::dvec3(-1, 1,-1),
    glm::dvec3(-1, 1, 1),
    glm::dvec3( 1,-1,-1),
    glm::dvec3( 1,-1, 1),
    glm::dvec3( 1, 1,-1),
    glm::dvec3( 1, 1, 1),
};
int face_indices[] = {
    0, 1, 3, 2,
    4, 5, 7, 6,
    0, 1, 5, 4, 
    2, 3, 7, 6, 
    0, 2, 6, 4,
    1, 3, 7, 5,
};

#define SET_SIZE(a, size) do{delete[] a; typedef typeof(*a) T; a=new T[size];}while(0)

void load_scene(const char * file) {
    for (int i=0; i<GRID_SIZE; i++) {
        grid[4*i+0]={-GRID_SIZE/2,0,(short)(i-GRID_SIZE/2)};
        grid[4*i+1]={+GRID_SIZE/2,0,(short)(i-GRID_SIZE/2)};
        grid[4*i+2]={(short)(i-GRID_SIZE/2),0,-GRID_SIZE/2};
        grid[4*i+3]={(short)(i-GRID_SIZE/2),0, GRID_SIZE/2};
    }
    position = glm::dvec3(0, PLAYER_HEIGHT, 0);
    
    filemap<block> blockfile(file);
    blocks = blockfile.length;
    SET_SIZE(block_coords, blocks*8);
    SET_SIZE(block_indices, blocks*24);
    SET_SIZE(block_collisions, blocks);
    SET_SIZE(collision_nodes, blocks);
    for (uint i=0; i<blocks; i++) {
        for (uint j=0; j<24; j++) {
            block_indices[i*24+j] = face_indices[j] + i*8;
        }
        const block &b = blockfile.list[i];
        double sizey = b.top/2;
        double posy = b.bottom + b.top/2;
        glm::dmat3 rotation(glm::rotate(glm::dmat4(), b.rotation*M_PI/180, glm::dvec3(0,1,0)));
        glm::dvec3 pos = glm::dvec3(b.posx, posy, b.posz);
        glm::dvec3 size = glm::dvec3(b.sizex, sizey, b.sizez);
        glm::dvec3 r_pos = glm::transpose(rotation)*pos;
        block_collisions[i].rotation = rotation;
        block_collisions[i].lb = r_pos-size-0.001;
        block_collisions[i].ub = r_pos+size+0.001;
        for (uint j=0; j<8; j++) {
            glm::dvec3 coord = rotation*(cube_coords[j]*size) + pos;
            block_coords[i*8+j].x = coord.x;
            block_coords[i*8+j].y = coord.y;
            block_coords[i*8+j].z = coord.z;
            block_coords[i*8+j].color = ((b.color>>16)&0xff) | (b.color&0xff00) | ((b.color&0xff)<<16);
        }
        printf("%6.3f %6.3f %6.3f %06x\n", b.posx, posy, b.posz, b.color);
    }
}

void draw() {
    glColor4f(0,1,0, 0.3);
    glVertexPointer(3,GL_SHORT,sizeof(*grid),grid);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_LINES, 0, GRID_SIZE*4);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3,GL_FLOAT,sizeof(*block_coords),&block_coords->x);
    glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(*block_coords),&block_coords->color);
    glDrawElements(GL_QUADS, blocks*24, GL_UNSIGNED_SHORT, block_indices);
    glDisableClientState(GL_COLOR_ARRAY);

    glColor3f(1,0,0);
    glPointSize(5.0);
    glVertexPointer(3,GL_FLOAT,sizeof(*collision_nodes),collision_nodes);
    glDrawArrays(GL_POINTS, 0, blocks);
}

void interact() {
    airborne = true;
    
    // Block collision.
    for (uint i=0; i<blocks; i++) {
        const block_collision &c = block_collisions[i];
        point3f & n = collision_nodes[i];
        glm::dvec3 projected = c.rotation * glm::min(c.ub,glm::max(c.lb,glm::transpose(c.rotation)*position));
        n.x = projected.x;
        n.y = projected.y;
        n.z = projected.z;
        glm::dvec3 dist = projected - position;
        double d = glm::dot(dist,dist);
        if (1e-3 < d && d <= PLAYER_SIZE*PLAYER_SIZE) {
            d = sqrt(d);
            dist /= d;
            // Move out of cube
            position -= dist*(PLAYER_SIZE-d-1e-3);
            // Fix velocity.
            velocity -= dist*glm::dot(dist, velocity);
            // Walking?
            if (dist.y<-0.8) {
                airborne = false;
            }
        }
    }

    // Ground collision.
    if (position.y <= PLAYER_HEIGHT + 1e-3) {
        position.y = PLAYER_HEIGHT;
        velocity.y = 0;
        airborne = false;
    }
}
