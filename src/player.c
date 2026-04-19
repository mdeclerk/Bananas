#include <gb/gb.h>
#include <string.h>
#include "viewport.h"
#include "player.h"
#include "terrain.h"
#include "oam.h"
#include "gfx.h"

#define PLAYER_AIM_STEP        2
#define PLAYER_START_AIM_REACH 35
#define PLAYER_SPRITE_COUNT    9
#define SHAKE_OFFSET           1

void player_init(player_t *self, player_type_t type) {
    memset(self, 0, sizeof(player_t));
    self->type = type;
}

void player_reset_round(player_t *self, const terrain_t *terrain) {
    self->aim.x = (self->type == PLAYER_LEFT) ? PLAYER_START_AIM_REACH : -PLAYER_START_AIM_REACH;
    self->aim.y = -PLAYER_START_AIM_REACH;
    self->lives = PLAYER_START_LIVES;
    self->shake = false;
    player_reset_position(self, terrain);
}

void player_set_shake(player_t *self, bool enable) {
    self->shake = enable;
}

void player_update_view(player_t *self, const viewport_t *viewport) {
    uint8_t oam = (self->type == PLAYER_LEFT) ? OAM_PLAYER0 : OAM_PLAYER1;
    vec2_t screen = viewport_to_screen(viewport, self->pos);

    if (screen.x < -PLAYER_BODY_W || screen.x >= SCREEN_W_PX ||
        screen.y < -PLAYER_BODY_H || screen.y >= SCREEN_H_PX) {
        oam_hide(oam, PLAYER_SPRITE_COUNT);
        return;
    }
        
    uint8_t shake_offset = 0;
    if (self->shake) {
        shake_offset = (sys_time & 1u) ? SHAKE_OFFSET : -SHAKE_OFFSET;
    }

    uint8_t sx = (uint8_t)screen.x + OAM_X_OFS + shake_offset;
    uint8_t sy = (uint8_t)screen.y + OAM_Y_OFS;

    bool flip_x = (self->type == PLAYER_RIGHT);
    uint8_t props = flip_x ? S_FLIPX : 0u;

    for (uint8_t row = 0, idx = 0; row < 3; row++) {
        for (uint8_t col = 0; col < 3; col++, idx++) {
            uint8_t tile_col = flip_x ? (2u - col) : col;
            uint8_t tile_idx = row * 3u + tile_col;

            set_sprite_tile(oam + idx, TILE_SPR_PLAYER_BASE + tile_idx);
            set_sprite_prop(oam + idx, props);
            move_sprite    (oam + idx, sx + col * 8, sy + row * 8);
        }
    }
}

void player_adjust_aim(player_t *self, vec2_t delta) {
    self->aim.x += delta.x * PLAYER_AIM_STEP;
    self->aim.y += delta.y * PLAYER_AIM_STEP;
}

vec2_t player_projectile_spawn(const player_t *self) {
    vec2_t spawn = vec2(
        self->type == PLAYER_LEFT ? self->pos.x + PLAYER_BODY_W : self->pos.x - 1,
        self->pos.y + 8
    );
    return spawn;
}

void player_take_damage(player_t *self) {
    if (self->lives > 0) {
        self->lives--;
    }
}

bool player_hit_test(const player_t *self, vec2_t pos) {
    return pos.x >= self->pos.x &&
           pos.x < (int16_t)(self->pos.x + PLAYER_BODY_W) &&
           pos.y >= self->pos.y &&
           pos.y < (int16_t)(self->pos.y + PLAYER_BODY_H);
}

void player_reset_position(player_t *self, const terrain_t *terrain) {
    uint8_t plateau = (self->type == PLAYER_LEFT) ? 0 : (terrain->plateau_count - 1);
    self->pos = vec2(
        plateau * PLATEAU_WIDTH * 8 + (PLATEAU_WIDTH * 8 / 2) - (PLAYER_BODY_W / 2), 
        (SCREEN_ROWS - terrain->heights[plateau]) * 8 - PLAYER_BODY_H
    );
}
