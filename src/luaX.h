#ifndef LUAX_H
#define LUAX_H

#include <glm/glm.hpp>
struct lua_State;


bool luaX_check_field(lua_State * L, int index, const char * field);
glm::dvec3 luaX_get_vector(lua_State * L);

#endif
