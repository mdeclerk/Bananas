#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "math.h"
#include "viewport.h"
#include "player.h"
#include "terrain.h"

typedef enum {
    PROJECTILE_NONE = 0,
    PROJECTILE_FLYING,
    PROJECTILE_HIT_TERRAIN,
    PROJECTILE_HIT_PLAYER,
    PROJECTILE_OOB
} projectile_state_t;

typedef struct {
    vec2fixed_t        pos;
    vec2fixed_t        vel;
    projectile_state_t state;
    uint8_t            age;
} projectile_t;

void               projectile_init(projectile_t *self);
void               projectile_launch(projectile_t *self, const player_t *shooter);
projectile_state_t projectile_simulate(projectile_t *self, const terrain_t *terrain, const player_t *target);
void               projectile_hide_view(projectile_t *self);
void               projectile_update_view(const projectile_t *self, const viewport_t *viewport);

#endif
