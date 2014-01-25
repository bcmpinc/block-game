#include <SDL/SDL.h>

#include "art.h"
#include "events.h"
#include "timing.h"
#include "scene.h"

int main (int argc, char *argv[]) {
    init_screen("blockgame");  
    
    load_scene("../maps/lvl_0000.blm");
    
    // mainloop
    while (!quit) {
        Timer t;
        if (moves) {
            interact();
            clear_screen();
            set_matrix();
            draw();
            flip_screen();
            printf("%c %6.3f %6.3f %6.3f\n", airborne?'A':'W', position.x, position.y, position.z);
        }
        next_frame(t.elapsed());
        handle_events();
    }
    
    return 0;
}
