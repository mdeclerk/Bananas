#include <gb/gb.h>
#include <rand.h>

#include "gfx.h"
#include "input.h"
#include "splash.h"
#include "generated/splash.h"

#define MSG_COL      5   /* (20-11)/2 rounded up to visually center */
#define MSG_ROW      15

#define SPLASH_COLS  (splash_WIDTH / 8)      /* 16 */
#define SPLASH_ROWS  (splash_HEIGHT / 8)     /* 12 */
#define SPLASH_X     ((20 - SPLASH_COLS) / 2)
#define SPLASH_Y     2

#define BLINK_ON_FRAMES   60   /* ≈ 1 s   */
#define BLINK_OFF_FRAMES  30   /* = 0.5 s */
#define BLINK_CYCLE       (BLINK_ON_FRAMES + BLINK_OFF_FRAMES)

static void draw_splash_image(void) {
    for (uint8_t row = 0; row < SPLASH_ROWS; row++) {
        uint8_t tile_row[SPLASH_COLS];
        for (uint8_t col = 0; col < SPLASH_COLS; col++) {
            tile_row[col] = TILE_BKG_SPLASH_BASE + row * SPLASH_COLS + col;
        }
        set_bkg_tiles(SPLASH_X, SPLASH_Y + row, SPLASH_COLS, 1, tile_row);
    }
}

void splash_screen(void) {
    HIDE_WIN;
    HIDE_SPRITES;

    gfx_clear_bkg();

    draw_splash_image();
    gfx_print_bkg(MSG_COL, MSG_ROW, "PRESS START");

    SHOW_BKG;
    DISPLAY_ON;

    for (;;) {
        vsync();
        input_update();

        if (input_pressed(J_START)) {
            initrand(((uint16_t)DIV_REG << 8) | DIV_REG);
            return;
        }

        uint8_t t = sys_time % BLINK_CYCLE;
        if (t == 0)
            gfx_print_bkg(MSG_COL, MSG_ROW, "PRESS START");
        else if (t == BLINK_ON_FRAMES)
            gfx_print_bkg(MSG_COL, MSG_ROW, "           ");
    }
}
