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
#include <physfs.h>
#include <unistd.h>
#include <sys/stat.h>

#include "art.h"
#include "events.h"
#include "timing.h"
#include "scene.h"

static bool select_paths() {
    char path[1024]; path[1023]=0;
    snprintf(path, 1023, "%s/.blockgame", PHYSFS_getUserDir());
    mkdir(path, 0755);
    if (!PHYSFS_setWriteDir(path)) {
        fprintf(stderr, "Failed to set write dir: %s\n", PHYSFS_getLastError());
        abort();
    }
    PHYSFS_mount(path, "/records/", 1);
    if (access("maps", R_OK | X_OK) == 0) {
        PHYSFS_mount(".", "/", 1);
        return true;
    } 
    if (access("../maps", R_OK | X_OK) == 0) {
        PHYSFS_mount("..", "/", 1);
        return true;
    }
    const char * p = PHYSFS_getBaseDir();
    snprintf(path, 1023, "%s/maps", p);
    if (access(path, R_OK | X_OK) == 0) {
        PHYSFS_mount(p, "/", 1);
        return true;
    } 
    snprintf(path, 1023, "%s/../maps", p);
    if (access("../maps", R_OK | X_OK) == 0) {
        snprintf(path, 1023, "%s/..", p);
        PHYSFS_mount(path, "/", 1);
        return true;
    }
    return false;
}

int main (int argc, char *argv[]) {
    // Initialize filesystem
    PHYSFS_init(argv[0]);
    if (!select_paths()) {
        fprintf(stderr, "Failed to determine path to data files.\n");
        return 1;
    }
    
    // Open initial map
    if (argc==2) {
        if (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0) {
            printf("Usage: %s [initial_map]\n", argv[0]);
            return 1;
        }
        if (!scene::load(argv[1])) {
            fprintf(stderr, "Failed to load map '%s'\n", argv[1]);
            return 1;
        }
    } else {
        if (!scene::load("maps/lvl_0000.map")) {
            fprintf(stderr, "Failed to load initial map.\n");
            return 1;
        }
    }
    
    init_screen("blockgame");  
    
    // mainloop
    while (!quit) {
        Timer t;
        scene::interact();
        clear_screen();
        set_matrix();
        scene::draw();
        flip_screen();
        next_frame(t.elapsed());
        handle_events();
    }
    scene::unload();
    
    return 0;
}
