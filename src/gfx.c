#include <gb/gb.h>
#include <string.h>

#include "gfx.h"
#include "viewport.h"
#include "terrain.h"
#include "generated/banana.h"
#include "generated/crosshair.h"
#include "generated/glyphs.h"
#include "generated/hud.h"
#include "generated/kong.h"
#include "generated/splash.h"
#include "generated/terrain.h"

static void init_palettes(void) {
    BGP_REG  = 0xE4;
    OBP0_REG = 0xE4;
    OBP1_REG = 0xE4;
}

void gfx_init(void) {
    init_palettes();

    set_sprite_data(TILE_SPR_PLAYER_BASE, kong_TILE_COUNT, kong_tiles);
    set_sprite_data(TILE_SPR_PROJECTILE_BASE, banana_TILE_COUNT, banana_tiles);
    set_sprite_data(TILE_SPR_CROSSHAIR_DOT, crosshair_TILE_COUNT, crosshair_tiles);

    set_bkg_data(TILE_WIN_FONT_BASE, glyphs_TILE_COUNT, glyphs_tiles);
    set_bkg_data(TILE_WIN_HUD_BASE, hud_TILE_COUNT, hud_tiles);
   
    set_bkg_data(TILE_BKG_TERRAIN_BASE, terrain_TILE_COUNT, terrain_tiles);
    set_bkg_data(TILE_BKG_SPLASH_BASE, splash_TILE_COUNT, splash_tiles);
}

void gfx_clear_bkg(void) {
    uint8_t blank[SCREEN_COLS];
    memset(blank, TILE_WIN_FONT_BASE + GLYPH_SPACE, sizeof(blank));
    for (uint8_t y = 0; y < SCREEN_ROWS; y++) {
        set_bkg_tiles(0, y, SCREEN_COLS, 1, blank);
    }
}

static uint8_t ascii_to_glyph(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    switch (c) {
        case 'P': return GLYPH_P;
        case 'R': return GLYPH_R;
        case 'E': return GLYPH_E;
        case 'S': return GLYPH_S;
        case 'T': return GLYPH_T;
        case 'A': return GLYPH_A;
        default:  return GLYPH_SPACE;
    }
}

void gfx_print_bkg(uint8_t x, uint8_t y, const char *str) {
    uint8_t buf[SCREEN_COLS];
    uint8_t len = 0;
    while (*str && len < SCREEN_COLS) {
        buf[len++] = TILE_WIN_FONT_BASE + ascii_to_glyph(*str++);
    }
    set_bkg_tiles(x, y, len, 1, buf);
}

void gfx_print_win(uint8_t x, uint8_t y, const char *str) {
    uint8_t buf[SCREEN_COLS];
    uint8_t len = 0;
    while (*str && len < SCREEN_COLS) {
        buf[len++] = TILE_WIN_FONT_BASE + ascii_to_glyph(*str++);
    }
    set_win_tiles(x, y, len, 1, buf);
}


