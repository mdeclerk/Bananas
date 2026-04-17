#include <gb/gb.h>

#include "oam.h"

void oam_hide(uint8_t first_id, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        move_sprite(first_id + i, 0, 0);
    }
}

void oam_move_to(uint8_t sprite_id, vec2_t world, const viewport_t *viewport) {
    vec2_t screen = viewport_to_screen(viewport, world);

    if (screen.x < -SPRITE_PX || screen.x >= SCREEN_W_PX ||
        screen.y < -SPRITE_PX || screen.y >= SCREEN_H_PX) {
        move_sprite(sprite_id, 0, 0);
        return;
    }

    move_sprite(
        sprite_id,
        (uint8_t)(screen.x + OAM_X_OFS - SPRITE_HALF),
        (uint8_t)(screen.y + OAM_Y_OFS - SPRITE_HALF)
    );
}
