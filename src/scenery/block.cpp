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
    glm::dvec3 velocity;
    glm::dvec3 size;
    glm::dmat3 rotation;
    glm::dmat3 rotational_velocity;
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
    void clear() {
        info.clear();
        coordinates.clear();
        face_indices.clear();
        wire_indices.clear();
        collision_nodes.clear();
        blocks = 0;
    }
};

struct object {
    glm::dvec3 offset;
    glm::dvec3 velocity;
    glm::dmat3 rotation;
    glm::dmat3 rotational_velocity;
    std::vector<int> entries;
    
    glm::dvec3 base_offset;
    std::vector<glm::dvec3> base_position;
    std::vector<glm::dmat3> base_rotation;
};

static block_container container;
static std::vector<object> objects;

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

// place_block(info{pos, vel, size, color}) : id;
static int place_block(lua_State * L) {
    block_info info;

    if (luaX_check_field(L, 1, "pos")) {
        info.position = luaX_get_vector(L);
    }

    if (luaX_check_field(L, 1, "vel")) {
        info.velocity = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 1, "size")) {
        info.size = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 1, "color")) {
        info.color = lua_tointeger(L, -1);
        lua_pop(L, 1);
    } else {
        info.color = 0xffffff;
    }
    
    // Create entries.
    int i = container.blocks;
    for (uint j=0; j<24; j++) {
        container.face_indices.push_back(face_indices[j] + i*8);
        container.wire_indices.push_back(wire_indices[j] + i*8);
    }
    container.info.push_back(info);
    container.collision_nodes.push_back(point3f());
    container.coordinates.resize(container.coordinates.size()+8);
    container.blocks++;
    
    // Update computed values.
    container.recompute(i);
    
    // Return the block (as index)
    lua_pushnumber(L, i);
    return 1;
}

// move_block(id, info{pos, vel, size, color});
static int move_block(lua_State * L) {
    unsigned int i = lua_tointeger(L, 1);
    luaL_argcheck(L, i<container.blocks, 1, "Block id out of range.");
    block_info &info = container.info[i];

    if (luaX_check_field(L, 2, "pos")) {
        info.position = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 2, "vel")) {
        info.velocity = luaX_get_vector(L);
    }

    if (luaX_check_field(L, 2, "size")) {
        info.size = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 2, "color")) {
        info.color = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    
    // Update computed values.
    container.recompute(i);
    
    return 0;
}

// rotate_block(id, {axis, angle(deg), angle_vel(deg), reset})
static int rotate_block(lua_State * L) {
    unsigned int i = lua_tointeger(L, 1);
    luaL_argcheck(L, i<container.blocks, 1, "Block id out of range.");
    block_info &info = container.info[i];

    bool ok = false;
    bool reset = false;

    if (luaX_check_field(L, 2, "reset")) {
        reset = true;
    }
    
    glm::dvec3 axis(0,1,0);
    if (luaX_check_field(L, 2, "axis")) {
        axis = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 2, "angle")) {
        double angle = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm::dmat3 rot(glm::rotate(glm::dmat4(), angle, axis));
        if (reset) info.rotation = rot;
        else info.rotation = rot * info.rotation;
        ok = true;
        
        // Update computed values.
        container.recompute(i);
    }

    if (luaX_check_field(L, 2, "angle_vel")) {
        double angle_vel = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm::dmat3 rot_vel(glm::rotate(glm::dmat4(), angle_vel, axis));
        if (reset) info.rotational_velocity = rot_vel;
        else info.rotational_velocity = rot_vel * info.rotational_velocity;
        ok = true;
    }

    luaL_argcheck(L, ok, 2, "rotation requires either 'angle' or 'angle_vel'.");
    return 0;
}

// create_object(blocks(list), origin)
int create_object(lua_State* L) {
    int r = objects.size();
    objects.resize(objects.size()+1);
    object & obj = objects.back();
    
    int n = luaL_getn(L, 1); // Should be luaL_objlen
    if (lua_gettop(L) >= 2) {
        lua_settop(L,2);
        obj.base_offset = obj.offset = luaX_get_vector(L);
    }
    
    for (int i=0; i<n; i++) {
        lua_rawgeti(L, 1, i+1);
        luaL_argcheck(L, lua_isnumber(L, 2), 1, "blocks list contains non-block");
        unsigned int id = lua_tointeger(L, -1);
        lua_pop(L, 1);
        luaL_argcheck(L, id<container.blocks, 1, "blocks list contains invalid ID");
        obj.entries.push_back(id);
        obj.base_position.push_back(container.info[id].position - obj.offset);
        obj.base_rotation.push_back(container.info[id].rotation);
    }
    
    lua_pop(L,1);
    lua_pushnumber(L, r);
    return 1;
}

// move_object(id, {offset, acceleration, reset})
int move_object(lua_State* L) {
    unsigned int obj_id = lua_tointeger(L, 1);
    luaL_argcheck(L, obj_id<objects.size(), 1, "Object id out of range.");
    object & obj = objects[obj_id];

    bool ok = false;
    bool reset = false;

    if (luaX_check_field(L, 2, "reset")) {
        reset = true;
    }
    
    if (luaX_check_field(L, 2, "offset")) {
        glm::dvec3 offset = luaX_get_vector(L);
        if (reset) obj.offset = obj.base_offset + offset;
        else obj.offset += offset;
        ok = true;
    }

    if (luaX_check_field(L, 2, "acceleration")) {
        glm::dvec3 acceleration = luaX_get_vector(L);
        if (reset) obj.velocity = acceleration;
        else obj.velocity += acceleration;
        ok = true;
    }

    luaL_argcheck(L, ok, 2, "move requires either 'offset' or 'acceleration'.");
    
    return 0;
}

// rotate_object(id, {axis, angle(deg), angle_vel(deg), reset})
static int rotate_object(lua_State * L) {
    unsigned int obj_id = lua_tointeger(L, 1);
    luaL_argcheck(L, obj_id<objects.size(), 1, "Object id out of range.");
    object & obj = objects[obj_id];

    bool ok = false;
    bool reset = false;

    if (luaX_check_field(L, 2, "reset")) {
        reset = true;
    }
    
    glm::dvec3 axis(0,1,0);
    if (luaX_check_field(L, 2, "axis")) {
        axis = luaX_get_vector(L);
    }
    
    if (luaX_check_field(L, 2, "angle")) {
        double angle = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm::dmat3 rot(glm::rotate(glm::dmat4(), angle, axis));
        if (reset) obj.rotation = rot;
        else obj.rotation = rot * obj.rotation;
        ok = true;
    }

    if (luaX_check_field(L, 2, "angle_vel")) {
        double angle_vel = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm::dmat3 rot_vel(glm::rotate(glm::dmat4(), angle_vel, axis));
        if (reset) obj.rotational_velocity = rot_vel;
        else obj.rotational_velocity = rot_vel * obj.rotational_velocity;
        ok = true;
    }

    luaL_argcheck(L, ok, 2, "rotation requires either 'angle' or 'angle_vel'.");
    return 0;
}

// update_object(id)
int update_object(lua_State* L) {
    unsigned int obj_id = lua_tointeger(L, 1);
    luaL_argcheck(L, obj_id<objects.size(), 1, "Object id out of range.");
    object & obj = objects[obj_id];
    
    int n = obj.entries.size();
    for (int i=0; i<n; i++) {
        block_info &block = container.info[obj.entries[i]];
        glm::dvec3 rel_pos = obj.rotation * obj.base_position[i];
        block.position = rel_pos + obj.offset;
        block.rotation = obj.rotation * obj.base_rotation[i];
        block.velocity = obj.velocity + obj.rotational_velocity*rel_pos - rel_pos;
        block.rotational_velocity = obj.rotational_velocity;
        container.recompute(obj.entries[i]);
    }
    
    return 0;
}

template<>
void scenery<blocks>::init(lua_State* L) {
    lua_register(L, "place_block",   place_block);
    lua_register(L, "move_block",    move_block);
    lua_register(L, "rotate_block",  rotate_block);
    lua_register(L, "create_object", create_object);
    lua_register(L, "update_object", update_object);
    lua_register(L, "move_object",   move_object);
    lua_register(L, "rotate_object", rotate_object);
}

template<>
void scenery<blocks>::clear() {
    container.clear();
    objects.clear();
}

template<>
void scenery<blocks>::draw() {
    // Cubes
    container.coordinates.data()->attach();
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);
    glDrawElements(GL_QUADS, container.blocks*24, GL_UNSIGNED_SHORT, container.face_indices.data());
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableClientState(GL_COLOR_ARRAY);
    
    // Cube outlines 
    glColor3f(0,0,0);
    glLineWidth(2);
    glDrawElements(GL_LINES, container.blocks*24, GL_UNSIGNED_SHORT, container.wire_indices.data());

    container.collision_nodes.data()->attach();
    glColor3f(0,0,0);
    glPointSize(5.0);
    glDrawArrays(GL_POINTS, 0, container.blocks);
    glColor3f(1,1,1);
    glPointSize(2.0);
    glDrawArrays(GL_POINTS, 0, container.blocks);
}

template<>
void scenery<blocks>::interact(lua_State*) {
    // Block collision.
    for (uint i=0; i<container.blocks; i++) {
        const block_info &c = container.info[i];
        point3f & n = container.collision_nodes[i];
        glm::dvec3 projected = c.rotation * glm::min(c.ub,glm::max(c.lb,position*c.rotation));
        n.x = projected.x;
        n.y = projected.y;
        n.z = projected.z;
        glm::dvec3 dist = projected - position;
        double d = glm::dot(dist,dist);
        if (1e-3 < d && d <= PLAYER_SIZE*PLAYER_SIZE) {
            // Normalize dist
            d = sqrt(d);
            dist /= d;
            
            // Move out of cube
            position -= dist*(PLAYER_SIZE-d-COLLISION_EPSILON);
            
            // Compute velocity of collision node
            glm::dvec3 cn_rel_pos = projected - c.position;
            glm::dvec3 cn_vel = c.velocity + c.rotational_velocity*cn_rel_pos - cn_rel_pos;
            
            // Change velocity to not be moving into the cube.
            velocity -= dist*std::max(glm::dot(dist, velocity-cn_vel), 0.0);
            
            // Walking?
            if (dist.y<-0.8) {
                airborne = false;
                ground_vel = cn_vel;
            }
        }
    }
}
