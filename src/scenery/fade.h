#ifndef SCENERY_FADE_H
#define SCENERY_FADE_H

enum class FADE_STATES {
    NONE,
    FADE_OUT,
    BLACK,
    FADE_IN
};

extern FADE_STATES fade_state;
extern void (*fade_action)(struct lua_State*);

#endif 
