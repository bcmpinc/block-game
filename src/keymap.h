#define QWERTY 1
#define DVORAK 2

// Specify keyboard here:
#define KEYBOARD QWERTY 

#if KEYBOARD == QWERTY
# define KEY_FORWARD  SDLK_w
# define KEY_BACKWARD SDLK_s
# define KEY_LEFT     SDLK_a
# define KEY_RIGHT    SDLK_d
# define KEY_JUMP     SDLK_SPACE
# define KEY_ADVANCE  SDLK_LSHIFT
# define KEY_REWIND   SDLK_LCTRL
#endif

#if KEYBOARD == DVORAK
# define KEY_FORWARD  SDLK_COMMA
# define KEY_BACKWARD SDLK_o
# define KEY_LEFT     SDLK_a
# define KEY_RIGHT    SDLK_e
# define KEY_JUMP     SDLK_SPACE
# define KEY_ADVANCE  SDLK_LSHIFT
# define KEY_REWIND   SDLK_LCTRL
#endif