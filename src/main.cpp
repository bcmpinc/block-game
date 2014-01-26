#include <SDL/SDL.h>

#include "art.h"
#include "events.h"
#include "timing.h"
#include "scene.h"

int main (int argc, char *argv[]) {
    int map_current=-1;
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
