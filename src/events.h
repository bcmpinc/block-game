/*
    Block Game - A minimalistic 3D platform game
    Copyright (C) 2013,2014  B.J. Conijn <bcmpinc@users.sourceforge.net>

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

#ifndef EVENTS_H
#define EVENTS_H
#include <glm/glm.hpp>

void handle_events();
void next_frame(int elapsed);
void reset(glm::dvec3 start_position);
void finish(glm::dvec3 end_position, const char * target_file);

extern bool quit;
extern bool airborne;
extern glm::dvec3 ground_vel;
extern glm::dmat3 orientation;
extern glm::dvec3 position;
extern glm::dvec3 velocity;
extern uint move_counter;

#endif
