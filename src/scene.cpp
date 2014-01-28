/*
    Block Game - A minimalistic 3D platform game
    Copyright (C) 2014  B.J. Conijn <bcmpinc@users.sourceforge.net>

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

#include <cstdint>
#include <vector>
#include <GL/gl.h>

#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene.h"
#include "events.h"
#include "filemap.h"
#include "point_types.h"

struct block {
    float posz;
    float posx;
    float sizex;
    float sizez;
    float height;
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
static const int GRID_SIZE = 1023;
static const double COLLISION_EPSILON = 0.002;
static const int END_DURATION = 60;
static const int ENEMY_TRAIL = 128;

static point3s grid[GRID_SIZE * 4];
static point3fc * block_coords;
static unsigned short * block_face_indices;
static unsigned short * block_wire_indices;
static block_collision * block_collisions;
static point3f * collision_nodes;
static unsigned int blocks;
static glm::dvec3 gem_location;
static double gem_rotation;
static bool gem_taken;
static int end_counter;
static filemap<point3f> enemy;
static char rec_filename[512];
static pointc not_yet_lost_color[ENEMY_TRAIL];
static pointc lost_color[ENEMY_TRAIL];

int map_next=0;

static glm::dvec3 cube_coords[] = {
    glm::dvec3(-1,-1,-1),
    glm::dvec3(-1,-1, 1),
    glm::dvec3(-1, 1,-1),
    glm::dvec3(-1, 1, 1),
    glm::dvec3( 1,-1,-1),
    glm::dvec3( 1,-1, 1),
    glm::dvec3( 1, 1,-1),
    glm::dvec3( 1, 1, 1),
};
static short face_indices[] = {
    1, 0, 2, 3,
    4, 5, 7, 6,
    0, 1, 5, 4, 
    3, 2, 6, 7, 
    2, 0, 4, 6,
    1, 3, 7, 5,
};
static short wire_indices[] = {
    0,1,0,2,0,4,
    1,3,1,5,
    2,3,2,6,
    3,7,
    4,5,4,6,
    5,7,
    6,7,
};

static point3f gem_coords[] = {
    {  0, .2,  0},
    {-.1,  0,  0},
    {  0,  0,-.1},
    { .1,  0,  0},
    {  0,  0, .1},
    {  0,-.2,  0},
};
static short gem_face_indices[] = {
    0, 1, 2, 2, 1, 5,
    0, 2, 3, 3, 2, 5,
    0, 3, 4, 4, 3, 5,
    0, 4, 1, 1, 4, 5,
};
static short gem_wire_indices[] = {
    0,1,0,2,0,3,0,4,
    1,2,2,3,3,4,4,1,
    1,5,2,5,3,5,4,5,
};
static point2s fade_coords[]= {
    {-1,-1},{1,-1},{1,1},{-1,1},  
};
#define SET_SIZE(a, size) do{delete[] a; typedef typeof(*a) T; a=new T[size];}while(0)

static bool not_yet_lost() {
   return enemy.fd==-1 || enemy.length>move_counter+8;   
}

void load_scene(const char * file) {
    for (int i=0; i<GRID_SIZE; i++) {
        grid[4*i+0]={-GRID_SIZE/2,0,(short)(i-GRID_SIZE/2)};
        grid[4*i+1]={+GRID_SIZE/2,0,(short)(i-GRID_SIZE/2)};
        grid[4*i+2]={(short)(i-GRID_SIZE/2),0,-GRID_SIZE/2};
        grid[4*i+3]={(short)(i-GRID_SIZE/2),0, GRID_SIZE/2};
    }
    reset(glm::dvec3(0, PLAYER_SIZE, 0));
    gem_taken = false;
    end_counter = END_DURATION;
    
    filemap<block> blockfile(file);
    blocks = blockfile.length;
    SET_SIZE(block_coords, blocks*8);
    SET_SIZE(block_face_indices, blocks*24);
    SET_SIZE(block_wire_indices, blocks*24);
    SET_SIZE(block_collisions, blocks);
    SET_SIZE(collision_nodes, blocks);
    for (uint i=0; i<blocks; i++) {
        for (uint j=0; j<24; j++) {
            block_face_indices[i*24+j] = face_indices[j] + i*8;
            block_wire_indices[i*24+j] = wire_indices[j] + i*8;
        }
        const block &b = blockfile.list[i];
        double sizey = b.height/2;
        double posy = b.bottom + b.height/2;
        glm::dmat3 rotation(glm::rotate(glm::dmat4(), b.rotation*M_PI/180, glm::dvec3(0,1,0)));
        glm::dvec3 pos = glm::dvec3(b.posx, posy, b.posz);
        glm::dvec3 size = glm::dvec3(b.sizex, sizey, b.sizez);
        glm::dvec3 r_pos = glm::transpose(rotation)*pos;
        block_collisions[i].rotation = rotation;
        block_collisions[i].lb = r_pos-size-COLLISION_EPSILON;
        block_collisions[i].ub = r_pos+size+COLLISION_EPSILON;
        for (uint j=0; j<8; j++) {
            glm::dvec3 coord = rotation*(cube_coords[j]*size) + pos;
            block_coords[i*8+j].x = coord.x;
            block_coords[i*8+j].y = coord.y;
            block_coords[i*8+j].z = coord.z;
            block_coords[i*8+j].color = 0xff000000 | ((b.color>>16)&0xff) | (b.color&0xff00) | ((b.color&0xff)<<16);
        }
        // printf("%6.3f %6.3f %6.3f %06x\n", b.posx, posy, b.posz, b.color);
        if (i==0) gem_location = pos + glm::dvec3(0,sizey+PLAYER_SIZE,0);
    }
    
    snprintf(rec_filename, sizeof(rec_filename), "%s.rec", file);
    enemy = filemap<point3f>(rec_filename);
    
    for (uint i=0; i<ENEMY_TRAIL; i++) {
        not_yet_lost_color[i].color = 0xffff00u + ((i*256/ENEMY_TRAIL)<<24u);
        lost_color[i].color = 0x0000ffu + ((i*256/ENEMY_TRAIL)<<24u);
    }
}

static void draw_grid() {
    grid->attach();
    glColor4f(0,1,0, 0.3);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1);
    glDrawArrays(GL_LINES, 0, GRID_SIZE*4);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

static void draw_blocks() {
    block_coords->attach();
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);
    glDrawElements(GL_QUADS, blocks*24, GL_UNSIGNED_SHORT, block_face_indices);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableClientState(GL_COLOR_ARRAY);
    glColor3f(0,0,0);
    glLineWidth(2);
    glDrawElements(GL_LINES, blocks*24, GL_UNSIGNED_SHORT, block_wire_indices);
}

static void draw_collision_markers() {
    collision_nodes->attach();
    glColor3f(0,0,0);
    glPointSize(5.0);
    glDrawArrays(GL_POINTS, 0, blocks);
    glColor3f(1,1,1);
    glPointSize(2.0);
    glDrawArrays(GL_POINTS, 0, blocks);
}

static void draw_gem() {
    gem_coords->attach();
    glPushMatrix();
    glTranslated(gem_location.x, gem_location.y, gem_location.z);
    glRotated(gem_rotation,0,1,0);
    if (not_yet_lost()) glColor3f(0,1,1);
    else glColor3f(1,0,0);
    glLineWidth(1);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, gem_wire_indices);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    if (not_yet_lost()) glColor4f(0,1,1,0.5);
    else glColor4f(1,0,0,0.5);
    glPolygonOffset(1,1);
    glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, gem_face_indices);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
    glPopMatrix();
}

static void draw_enemy() {
    glPushMatrix();
    glTranslated(0,-PLAYER_SIZE,0);
    enemy.list->attach();
    glLineWidth(3);
    int first = std::max(0, (int)move_counter-ENEMY_TRAIL);
    int last = std::min(enemy.length, move_counter);
    if (not_yet_lost()) not_yet_lost_color[ENEMY_TRAIL-(int)move_counter].attach();
    else lost_color[ENEMY_TRAIL-(int)move_counter].attach();
    glEnable(GL_BLEND);
    glEnableClientState(GL_COLOR_ARRAY);
    glDrawArrays(GL_LINE_STRIP, first, last-first);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_BLEND);
    glPopMatrix();
}

static void draw_end_fade() {
    fade_coords->attach();
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glColor4f(0,0,0,end_counter/(float)END_DURATION);
    glDrawArrays(GL_QUADS, 0, 4);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void draw() {
    draw_grid();
    draw_blocks();
    draw_collision_markers();
    if (!gem_taken) draw_gem();
    draw_enemy();
    if (end_counter>0) draw_end_fade();
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
            position -= dist*(PLAYER_SIZE-d-COLLISION_EPSILON);
            // Fix velocity.
            velocity -= dist*glm::dot(dist, velocity);
            // Walking?
            if (dist.y<-0.8) {
                airborne = false;
            }
        }
    }

    // Ground collision.
    if (position.y <= PLAYER_SIZE + 1e-3) {
        position.y = PLAYER_SIZE;
        velocity.y = 0;
        airborne = false;
    }
    
    // Gem
    if (gem_taken) {
        end_counter++;
        if (end_counter>=END_DURATION){
            map_next++;
        }
    } else {
        gem_rotation += 5;
        glm::dvec3 gem_dist = gem_location - position;
        if (glm::dot(gem_dist,gem_dist) < PLAYER_SIZE*PLAYER_SIZE) {
            gem_taken = true;
            // printf("end: %d %d %d\n", enemy.fd, enemy.length, move_counter+8);
            if (not_yet_lost()) {
                finish(gem_location+glm::dvec3(0,PLAYER_SIZE,0), rec_filename);
            }
        } else if (end_counter>0) {
            end_counter--;
        }
    }
    
    // Recording
}
