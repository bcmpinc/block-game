#include <vector>
#include <GL/gl.h>
#include <lua.hpp>
#include <string.h>
#include <physfs.h>

#include "../point_types.h"
#include "../events.h"
#include "../luaX.h"
#include "scenery.h"
#include "fade.h"

struct gems;

static const int ENEMY_TRAIL = 128;
static pointc not_yet_lost_color[ENEMY_TRAIL];
static pointc lost_color[ENEMY_TRAIL];

struct gem {
    glm::dvec3 position;
    double rotation;
    bool taken;
    char record_file[64];
    std::vector<point3f> record;
    bool not_yet_lost() {
        return record.empty() || record.size() > move_counter+8;
    }
};

static std::vector<gem> gemlist;

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

// place_gem(data{pos, record, next})
static int place_gem(lua_State * L) {
    gem g;
    g.taken = false;
    g.rotation = 0;
    if (luaX_check_field(L, 1, "pos")) {
        g.position = luaX_get_vector(L);
    }
    if (luaX_check_field(L, 1, "record")) {
        size_t length;
        const char * record = luaL_checklstring(L, -1, &length);
        luaL_argcheck(L, length<64, 1, "record path too long (max. 63 characters)");
        memcpy(g.record_file, record, length+1);
        lua_pop(L, 1);
        
        char read_file[80];
        snprintf(read_file, 80, "records/%s", g.record_file);
        PHYSFS_File * r = PHYSFS_openRead(read_file);
        if (r) {
            int len = PHYSFS_fileLength(r) / sizeof(point3f);
            g.record.resize(len);
            PHYSFS_read(r, (void*)g.record.data(), sizeof(point3f), len);
            PHYSFS_close(r);
        }
    }
   
    gemlist.push_back(std::move(g));
    return 0;
}

template<>
void scenery<gems>::init(lua_State * L) {
    for (uint i=0; i<ENEMY_TRAIL; i++) {
        not_yet_lost_color[i].color = 0xffff00u + ((i*256/ENEMY_TRAIL)<<24u);
        lost_color[i].color = 0x0000ffu + ((i*256/ENEMY_TRAIL)<<24u);
    }
    lua_register(L, "place_gem",  place_gem);
}

template<>
void scenery<gems>::clear() {
    gemlist.clear();
}

template<>
void scenery<gems>::draw() {
    gem_coords->attach();
    
    // Draw gems outlines
    glLineWidth(1);
    for (gem & g : gemlist) {
        if (!g.taken) {
            glPushMatrix();
            glTranslated(g.position.x, g.position.y, g.position.z);
            glRotated(g.rotation,0,1,0);
            if (g.not_yet_lost()) glColor3f(0,1,1);
            else glColor3f(1,0,0);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, gem_wire_indices);
            glPopMatrix();
        }
    }

    // Draw gem faces
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);
    for (gem & g : gemlist) {
        if (!g.taken) {
            glPushMatrix();
            glTranslated(g.position.x, g.position.y, g.position.z);
            glRotated(g.rotation,0,1,0);
            if (g.not_yet_lost()) glColor4f(0,1,1,0.5);
            else glColor4f(1,0,0,0.5);
            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, gem_face_indices);
            glPopMatrix();
        }
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);

    // Draw records
    glLineWidth(5);
    int first = std::max(0, (int)move_counter-ENEMY_TRAIL);
    glEnable(GL_BLEND);
    glEnableClientState(GL_COLOR_ARRAY);
    point3fc sparks[64];
    float IRM = 1. / RAND_MAX;
    glDepthMask(false);
    for (gem & g : gemlist) {
        if (move_counter >= g.record.size()+ENEMY_TRAIL) continue;
        glPushMatrix();
        glTranslated(0,-PLAYER_SIZE,0);
        int last = std::min(g.record.size(), (size_t)move_counter);
        if (g.not_yet_lost()) not_yet_lost_color[ENEMY_TRAIL-(int)move_counter].attach();
        else lost_color[ENEMY_TRAIL-(int)move_counter].attach();
        g.record.data()->attach();
        glDrawArrays(GL_LINE_STRIP, first, last-first);
        
        if (0 < move_counter && move_counter <= g.record.size()) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glLineWidth(3);
            point3f pos = g.record[move_counter-1];
            glTranslated(pos.x,pos.y,pos.z);
            for (int i=0; i<32; i++) {
                uint base_color = rand()&0xffffff;
                sparks[i*2] = {0,0,0, 0xff000000 | base_color};
                float x,y,z;
                do {x=rand()*IRM-0.5;y=rand()*IRM-0.5;z=rand()*IRM-0.5;} while (x*x+y*y+z*z>0.25);
                sparks[i*2+1] = {x,y,z, base_color};
                
            }
            sparks->attach();
            glDrawArrays(GL_LINES, 0, 64);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        glPopMatrix();
    }
    glDepthMask(true);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_BLEND);
}

template<>
void scenery<gems>::interact() {
    for (gem &g : gemlist) {
        if (g.taken) {
            if (fade_state == FADE_STATES::BLACK) {
                // TODO: load new map.
            }
        } else {
            g.rotation += 5;
            glm::dvec3 gem_dist = g.position - position;
            if (glm::dot(gem_dist,gem_dist) < PLAYER_SIZE*PLAYER_SIZE) {
                g.taken = true;
                // TODO perform action
                fade_state = FADE_STATES::FADE_OUT;
                if (g.not_yet_lost()) {
                    finish(g.position + glm::dvec3(0,PLAYER_SIZE,0), g.record_file);
                }
            }
        }
    }
}

