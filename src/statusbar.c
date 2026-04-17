#include <gb/gb.h>
#include "gfx.h"
#include "statusbar.h"
#include "player.h"

#define STATUSBAR_ROW_LEN  20
#define STATUSBAR_WIN_Y   136
#define STATUSBAR_P1_LIVES_COL 3
#define STATUSBAR_P2_LIVES_COL (STATUSBAR_ROW_LEN - PLAYER_START_LIVES - 3)

static void statusbar_draw_lives(uint8_t x, uint8_t y, uint8_t lives) {
    uint8_t tiles[PLAYER_START_LIVES];
    for (uint8_t i = 0; i < PLAYER_START_LIVES; i++) {
        tiles[i] = lives > i ? TILE_WIN_HUD_HEART_FULL : TILE_WIN_HUD_HEART_EMPTY;
    }

    set_win_tiles(x, y, PLAYER_START_LIVES, 1, tiles);
}

static void statusbar_draw_score(uint8_t x, uint8_t y, uint8_t score) {
    uint8_t buf[2];
    buf[0] = TILE_WIN_FONT_BASE + GLYPH_0 + (score / 10);
    buf[1] = TILE_WIN_FONT_BASE + GLYPH_0 + (score % 10);
    set_win_tiles(x, y, 2, 1, buf);
}

void statusbar_init(void) {
    mode(get_mode() | M_NO_SCROLL);
    move_win(MINWNDPOSX, STATUSBAR_WIN_Y);
}

void statusbar_update_view(const player_t *players) {
    statusbar_draw_score(0, 0, players[0].score);
    statusbar_draw_score(18, 0, players[1].score);
    statusbar_draw_lives(STATUSBAR_P1_LIVES_COL, 0, players[0].lives);
    statusbar_draw_lives(STATUSBAR_P2_LIVES_COL, 0, players[1].lives);
}
