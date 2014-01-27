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

#include <SDL/SDL.h>

#include "art.h"
#include "events.h"
#include "timing.h"
#include "scene.h"

int main (int argc, char *argv[]) {
    int map_current=-1;
    if (argc==2) {
        sscanf(argv[1],"%u",&map_next);
    }
    init_screen("blockgame");  
    
    // mainloop
    while (!quit && map_next<=8) {
        Timer t;
        if (map_current != map_next) {
            char file[40];
            snprintf(file, 40, "../maps/lvl_%04d.blm", map_next);
            load_scene(file);
            map_current = map_next;
        }
        interact();
        clear_screen();
        set_matrix();
        draw();
        flip_screen();
        //printf("%c %6.3f %6.3f %6.3f\n", airborne?'A':'W', position.x, position.y, position.z);
        next_frame(t.elapsed());
        handle_events();
    }
    
    return 0;
}
