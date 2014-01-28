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

#ifndef POINT_TYPES_H
#define POINT_TYPES_H
struct point2s {
    short x,y;
    void attach() const;
};

struct point3s {
    short x,y,z;
    void attach() const;
};

struct point3f {
    float x,y,z;
    void attach() const;
};

struct point3fc {
    float x,y,z;
    uint32_t color;
    void attach() const;
};

struct pointc {
    uint32_t color;
    void attach() const;
};

#endif
