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

#include <GL/gl.h>
#include "point_types.h"

void point2s::attach() const {
    glVertexPointer(2,GL_SHORT,sizeof(*this),this);
}

void point3s::attach() const {
    glVertexPointer(3,GL_SHORT,sizeof(*this),this);
}

void point3f::attach() const {
    glVertexPointer(3,GL_FLOAT,sizeof(*this),this);
}

void point3fc::attach() const {
    glVertexPointer(3,GL_FLOAT,sizeof(*this),&this->x);
    glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(*this),&this->color);
}
