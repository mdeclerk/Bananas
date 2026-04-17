#include <gb/gb.h>
#include <string.h>

#include "oam.h"
#include "gfx.h"
#include "crosshair.h"

#define CROSSHAIR_POINT_COUNT OAM_CROSSHAIR_COUNT
#define CROSSHAIR_MARK_INDEX  ((uint8_t)(CROSSHAIR_POINT_COUNT - 1u))

void crosshair_hide_view(void) {
    oam_hide(OAM_CROSSHAIR_FIRST, OAM_CROSSHAIR_COUNT);
}

static uint8_t index_to_sprite_tile(uint8_t index) {
    if (index == CROSSHAIR_MARK_INDEX) {
        return TILE_SPR_CROSSHAIR_MARK;
    }

    return TILE_SPR_CROSSHAIR_DOT;
}

void crosshair_init(void) {
    for (uint8_t i = 0; i < OAM_CROSSHAIR_COUNT; i++) {
        set_sprite_tile(OAM_CROSSHAIR_FIRST + i, index_to_sprite_tile(i));
    }
}

void crosshair_update_view(const player_t *player, const viewport_t *viewport) {
    vec2_t spawn = player_projectile_spawn(player);

    for (uint8_t i = 0; i < CROSSHAIR_POINT_COUNT; i++) {
        vec2_t point = vec2(
            spawn.x + (player->aim.x * (i + 1)) / CROSSHAIR_POINT_COUNT,
            spawn.y + (player->aim.y * (i + 1)) / CROSSHAIR_POINT_COUNT
        );
        oam_move_to(OAM_CROSSHAIR_FIRST + i, point, viewport);
    }
}
