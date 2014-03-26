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

#include <lua.hpp>
#include <physfs.h>
#include <cstring>

#include "scene.h"
#include "events.h"
#include "scenery/scenery.h"
#include "scenery/fade.h"

struct grid;
struct blocks;
struct gems;
struct fade;

static char next_map[64];
static bool load_next_map = false;

static void do_load_map(lua_State * ) {
    load_next_map = true;
}

static int load_map(lua_State * L) {
    size_t length;
    const char * map = luaL_checklstring(L, -1, &length);
    luaL_argcheck(L, length<64, 1, "map path too long (max. 63 characters)");
    memcpy(next_map, map, length+1);
    lua_pop(L, 1);
    fade_state = FADE_STATES::FADE_OUT;
    fade_action = do_load_map;
    return 0;
}

static void do_quit_game(lua_State * ) {
    printf("Quit\n");
    quit = true;
}

static int quit_game(lua_State * ) {
    fade_state = FADE_STATES::FADE_OUT;
    fade_action = do_quit_game;
    return 0;
}

static char read_buffer[1024];
const char* read_physfs_file(lua_State *, void* data, size_t* size) {
    PHYSFS_File * script = (PHYSFS_File*)data;
    *size = PHYSFS_read(script, read_buffer, 1, 1024);
    return read_buffer;
}

static lua_State * scene_lua;

bool scene::load(const char* filename) {
    assert(scene_lua == NULL);
    scene_lua = luaL_newstate();
    scenery<grid>::init(scene_lua);
    scenery<blocks>::init(scene_lua);
    scenery<gems>::init(scene_lua);
    scenery<fade>::init(scene_lua);
    lua_register(scene_lua, "load_map", load_map);
    lua_register(scene_lua, "quit",     quit_game);
    reset(glm::dvec3(0, PLAYER_SIZE, 0));
    
    PHYSFS_File * script = PHYSFS_openRead(filename);
    if (!script) {
        fprintf(stderr, "Failed to open '%s'\n", filename);
        exit(1);
    }
    int err = lua_load(scene_lua, read_physfs_file, script, filename);
    if (err) {
        fprintf(stderr, "Failed to parse '%s': %s\n", filename, lua_tolstring(scene_lua, 1, NULL));
        exit(1);
    } else {
        err = lua_pcall(scene_lua, 0, LUA_MULTRET, 0);
        if (err) {
            fprintf(stderr, "Failed to execute '%s': %s\n", filename, lua_tolstring(scene_lua, 1, NULL));
            exit(1);
        }
    }
    PHYSFS_close(script);
    return true;
}

void scene::unload() {
    scenery<grid>::clear();
    scenery<blocks>::clear();
    scenery<gems>::clear();
    scenery<fade>::clear();
    lua_close(scene_lua);
    scene_lua = NULL;
}

void scene::draw() {
    scenery<grid>::draw();
    scenery<blocks>::draw();
    scenery<gems>::draw();
    scenery<fade>::draw();
}

void scene::interact() {
    if (load_next_map) {
        scene::unload();
        printf("Loading map %s\n", next_map);
        char next[80];
        snprintf(next, 80, "maps/%s", next_map);
        scene::load(next);
        load_next_map = false;
    }
    airborne = true;
    scenery<blocks>::interact(scene_lua);
    scenery<grid>::interact(scene_lua);
    scenery<gems>::interact(scene_lua);
    scenery<fade>::interact(scene_lua);
}
