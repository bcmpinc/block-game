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

#ifndef SCENERY_H
#define SCENERY_H

struct lua_State;

static const double PLAYER_SIZE = 0.4; 
static const double COLLISION_EPSILON = 0.002;

template<class T>
struct scenery {
    static void init(lua_State *);
    static void clear();
    static void draw();
    static void interact();
};

#endif