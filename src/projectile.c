#include <gb/gb.h>
#include <string.h>

#include "oam.h"
#include "gfx.h"
#include "projectile.h"

#define PROJECTILE_GRAVITY 24
#define PROJECTILE_SPEED_DIV 16
#define PROJECTILE_OOB_PAD_PX 16

void projectile_init(projectile_t *self) {
    memset(self, 0, sizeof(*self));
    projectile_hide_view(self);
}

void projectile_hide_view(projectile_t *self) {
    self->state = PROJECTILE_NONE;
    move_sprite(OAM_PROJECTILE, 0, 0);
}

void projectile_launch(projectile_t *self, const player_t *shooter) {
    vec2_t spawn = player_projectile_spawn(shooter);
    self->pos = vec2fixed(FIXED(spawn.x), FIXED(spawn.y));
    self->vel = vec2fixed(
        FIXED(shooter->aim.x) / PROJECTILE_SPEED_DIV, 
        FIXED(shooter->aim.y) / PROJECTILE_SPEED_DIV);
    self->state = PROJECTILE_FLYING;
    self->age = 0;
}

projectile_state_t projectile_simulate(projectile_t *self, const terrain_t *terrain, const player_t *target) {
    if (self->state != PROJECTILE_FLYING) {
        return self->state;
    }

    self->pos.x += self->vel.x;
    self->pos.y += self->vel.y;
    self->vel.y += PROJECTILE_GRAVITY;
    if (self->age < 255) {
        self->age++;
    }

    vec2_t pos = vec2(FIXED_INT(self->pos.x), FIXED_INT(self->pos.y));

    if (pos.x < -(int16_t)PROJECTILE_OOB_PAD_PX ||
        pos.x >= terrain->size_px.x + PROJECTILE_OOB_PAD_PX ||
        pos.y >= SCREEN_H_PX) {
        self->state = PROJECTILE_OOB;
        return self->state;
    }

    if (player_hit_test(target, pos)) {
        self->state = PROJECTILE_HIT_PLAYER;
        return self->state;
    }

    if (terrain_hit_test(terrain, pos)) {
        self->state = PROJECTILE_HIT_TERRAIN;
        return self->state;
    }

    return self->state;
}

void projectile_update_view(const projectile_t *self, const viewport_t *viewport) {
    if (self->state == PROJECTILE_FLYING) {
        set_sprite_tile(OAM_PROJECTILE,
                        TILE_SPR_PROJECTILE_BASE + ((self->age >> 2) & 3));
    } else if (self->state == PROJECTILE_HIT_TERRAIN || self->state == PROJECTILE_HIT_PLAYER) {
        set_sprite_tile(OAM_PROJECTILE, TILE_SPR_EXPLOSION);
    } else {
        return;
    }

    vec2_t world = vec2(FIXED_INT(self->pos.x), FIXED_INT(self->pos.y));
    oam_move_to(OAM_PROJECTILE, world, viewport);
}
