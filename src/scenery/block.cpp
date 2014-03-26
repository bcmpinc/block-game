#include <vector>
#include <GL/gl.h>
#include <lua.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../point_types.h"
#include "../events.h"
#include "../luaX.h"
#include "scenery.h"

struct blocks;

struct block_info {
    glm::dvec3 position;
    glm::dvec3 size;
    glm::dmat3 rotation;
    glm::dvec3 lb;
    glm::dvec3 ub;
    int color;
};

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

struct block_container {
    std::vector<block_info> info;
    std::vector<point3fc> coordinates;
    std::vector<unsigned short> face_indices;
    std::vector<unsigned short> wire_indices;
    std::vector<point3f> collision_nodes;
    unsigned int blocks;
    void recompute(unsigned int i) {
        assert(i<blocks);
        block_info &b = info[i];
        glm::dvec3 r_pos = glm::transpose(b.rotation)*b.position;
        b.lb = r_pos-b.size-COLLISION_EPSILON;
        b.ub = r_pos+b.size+COLLISION_EPSILON;
        for (uint j=0; j<8; j++) {
            glm::dvec3 coord = b.rotation*(cube_coords[j]*b.size) + b.position;
            coordinates[i*8+j].x = coord.x;
            coordinates[i*8+j].y = coord.y;
            coordinates[i*8+j].z = coord.z;
            coordinates[i*8+j].color = b.color;
        }
    }
};

static block_container * container;

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

// place_block(info{pos, size, color}) : id;
static int place_block(lua_State * L) {
    block_info info;

    if (luaX_check_field(L, 1, "pos")) {
        info.position = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 1, "size")) {
        info.size = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 1, "color")) {
        info.color = lua_tonumber(L, -1);
        lua_pop(L, 1);
    } else {
        info.color = 0xffffff;
    }
    
    // Create entries.
    int i = container->blocks;
    for (uint j=0; j<24; j++) {
        container->face_indices.push_back(face_indices[j] + i*8);
        container->wire_indices.push_back(wire_indices[j] + i*8);
    }
    container->info.push_back(info);
    container->collision_nodes.push_back(point3f());
    container->coordinates.resize(container->coordinates.size()+8);
    container->blocks++;
    
    // Update computed values.
    container->recompute(i);
    
    // Return the block (as index)
    lua_pushnumber(L, i);
    return 1;
}

// move_block(id, info{pos, size, color});
static int move_block(lua_State * L) {
    unsigned int i = lua_tonumber(L, 1);
    luaL_argcheck(L, i<container->blocks, 1, "Block id out of range.");
    block_info &info = container->info[i];

    if (luaX_check_field(L, 2, "pos")) {
        info.position = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 2, "size")) {
        info.size = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 2, "color")) {
        info.color = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    
    // Update computed values.
    container->recompute(i);
    
    return 0;
}

// rotate(id, {axis, angle(deg), reset})
static int rotate_block(lua_State * L) {
    unsigned int i = lua_tonumber(L, 1);
    luaL_argcheck(L, i<container->blocks, 1, "Block id out of range.");
    block_info &info = container->info[i];

    bool ok = false;

    if (luaX_check_field(L, 2, "reset")) {
        info.rotation = glm::dmat3();
        ok = true;
    }
    
    glm::dvec3 axis(0,1,0);
    if (luaX_check_field(L, 2, "axis")) {
        info.size = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 2, "angle")) {
        double angle = lua_tonumber(L, -1);
        lua_pop(L, 1);
        info.rotation *= glm::dmat3(glm::rotate(glm::dmat4(), angle, axis));
        ok = true;
    }

    luaL_argcheck(L, ok, 2, "rotation requires either 'reset' or 'angle'.");

    // Update computed values.
    container->recompute(i);
    return 0;
}

template<>
void scenery<blocks>::init(lua_State* L) {
    container = new block_container();
    lua_register(L, "place_block",  place_block);
    lua_register(L, "move_block",   move_block);
    lua_register(L, "rotate_block", rotate_block);
}

template<>
void scenery<blocks>::clear() {
    delete container;
    container = NULL;
}

template<>
void scenery<blocks>::draw() {
    // Cubes
    container->coordinates.data()->attach();
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);
    glDrawElements(GL_QUADS, container->blocks*24, GL_UNSIGNED_SHORT, container->face_indices.data());
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableClientState(GL_COLOR_ARRAY);
    
    // Cube outlines 
    glColor3f(0,0,0);
    glLineWidth(2);
    glDrawElements(GL_LINES, container->blocks*24, GL_UNSIGNED_SHORT, container->wire_indices.data());

    container->collision_nodes.data()->attach();
    glColor3f(0,0,0);
    glPointSize(5.0);
    glDrawArrays(GL_POINTS, 0, container->blocks);
    glColor3f(1,1,1);
    glPointSize(2.0);
    glDrawArrays(GL_POINTS, 0, container->blocks);
}

template<>
void scenery<blocks>::interact(lua_State*) {
    // Block collision.
    for (uint i=0; i<container->blocks; i++) {
        const block_info &c = container->info[i];
        point3f & n = container->collision_nodes[i];
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
}
