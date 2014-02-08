#include <lua.hpp>
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
