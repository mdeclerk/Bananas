#include "gfx.h"
#include "sfx.h"
#include "input.h"
#include "game.h"
#include "splash.h"

void main(void) {
    gfx_init();
    sfx_init();
    input_init();
    splash_screen();
    game_loop();
}
