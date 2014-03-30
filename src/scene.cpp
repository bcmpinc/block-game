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
#include "luaX.h"

struct grid;
struct blocks;
struct gems;
struct fade;

static char next_map[64];
static bool load_next_map = false;
static int lua_tick_function = LUA_REFNIL;
static PHYSFS_sint64 map_script_moddate;
static char script_file[256];

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

static lua_State * scene_lua;

static void obtain_lua_tick_function(lua_State * L) {
    lua_tick_function = LUA_REFNIL;
    lua_getglobal(L, "tick");
    if (lua_isnil(L, -1)) {
        lua_pop(L,1);
    } else if (lua_isfunction(L, -1)) {
        lua_tick_function = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        fprintf(stderr, "'tick' is not a valid function");
        lua_pop(L,1);
    }
}

static bool do_load(const char* filename, bool do_reset) {
    map_script_moddate = PHYSFS_getLastModTime(filename);
    strncpy(script_file, filename, 255);
    assert(scene_lua == NULL);
    scene_lua = luaL_newstate();
    luaopen_math(scene_lua);
    luaX_open_math_ext(scene_lua);
    scenery<grid>::init(scene_lua);
    scenery<blocks>::init(scene_lua);
    scenery<gems>::init(scene_lua);
    scenery<fade>::init(scene_lua);
    lua_register(scene_lua, "load_map", load_map);
    lua_register(scene_lua, "quit",     quit_game);
    if (do_reset) reset(glm::dvec3(0, PLAYER_SIZE, 0));
    if (!luaX_execute_script(scene_lua, filename)) {
        return false;
    }
    obtain_lua_tick_function(scene_lua);
    return true;
}

bool scene::load(const char* filename) {
    return do_load(filename, true);
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
    if (map_script_moddate != PHYSFS_getLastModTime(script_file)) {
        scene::unload();
        printf("Reloading %s\n", script_file);
        do_load(script_file, false);
    }
    if (lua_tick_function != LUA_REFNIL) {
        lua_rawgeti(scene_lua, LUA_REGISTRYINDEX, lua_tick_function);
        lua_pushinteger(scene_lua, move_counter);
        if (lua_pcall(scene_lua, 1, 0, 0) != 0) {
            fprintf(stderr, "error running tick function: %s\n", lua_tostring(scene_lua, -1));
            lua_pop(scene_lua, 1);
        }
    }
    
    airborne = true;
    scenery<blocks>::interact(scene_lua);
    scenery<grid>::interact(scene_lua);
    scenery<gems>::interact(scene_lua);
    scenery<fade>::interact(scene_lua);
}
