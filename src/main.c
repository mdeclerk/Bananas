#include "gfx.h"
#include "sfx.h"
#include "input.h"
#include "game.h"
#include "splash.h"

game_t g_game; // Make game state a global symbol in WRAM

void main(void) {
    gfx_init();
    sfx_init();
    input_init();
    splash_screen();
    game_loop(&g_game);
}
