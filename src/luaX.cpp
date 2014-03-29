#include <lua.hpp>
#include <physfs.h>
#include "luaX.h"

bool luaX_check_field(lua_State * L, int index, const char * field) {
    lua_getfield(L, index, field);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

glm::dvec3 luaX_get_vector(lua_State * L) {
    int index = lua_gettop(L);
    luaL_argcheck(L, lua_istable(L, index), index, "Argument must be a vector");
    lua_rawgeti(L, index, 1);
    lua_rawgeti(L, index, 2);
    lua_rawgeti(L, index, 3);
    glm::dvec3 r(lua_tonumber(L, -3), lua_tonumber(L, -2), lua_tonumber(L, -1));
    lua_pop(L, 4);
    return r;
}

static char read_buffer[1024];
static const char* read_physfs_file(lua_State *, void* data, size_t* size) {
    PHYSFS_File * script = (PHYSFS_File*)data;
    *size = PHYSFS_read(script, read_buffer, 1, 1024);
    return read_buffer;
}

bool luaX_execute_script(lua_State * L, const char * physfs_filename)
{
    PHYSFS_File * script = PHYSFS_openRead(physfs_filename);
    if (!script) {
        fprintf(stderr, "Failed to open '%s'\n", physfs_filename);
        PHYSFS_close(script);
        return false;
    }
    int err = lua_load(L, read_physfs_file, script, physfs_filename);
    if (err) {
        fprintf(stderr, "Failed to parse '%s': %s\n", physfs_filename, lua_tolstring(L, 1, NULL));
        PHYSFS_close(script);
        return false;
    } else {
        err = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (err) {
            fprintf(stderr, "Failed to execute '%s': %s\n", physfs_filename, lua_tolstring(L, 1, NULL));
            PHYSFS_close(script);
            return false;
        }
    }
    PHYSFS_close(script);
    return true;
}
