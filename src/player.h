#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <stdint.h>

#include "math.h"
#include "viewport.h"
#include "terrain.h"

#define PLAYER_START_LIVES  3
#define PLAYER_BODY_W       24
#define PLAYER_BODY_H       24

typedef enum {
    PLAYER_LEFT = 0,
    PLAYER_RIGHT = 1
} player_type_t;

typedef struct {
    player_type_t type;
    vec2_t        pos;
    vec2_t        aim;
    uint8_t       lives;
    uint8_t       score;
    bool          shake;
} player_t;

void    player_init(player_t *self, player_type_t type);
void    player_reset_round(player_t *self, const terrain_t *terrain);
void    player_update_view(player_t *self, const viewport_t *viewport);
void    player_adjust_aim(player_t *self, vec2_t delta);
vec2_t  player_projectile_spawn(const player_t *self);
void    player_take_damage(player_t *self);
bool    player_hit_test(const player_t *self, vec2_t pos);
void    player_reset_position(player_t *self, const terrain_t *terrain);
void    player_set_shake(player_t *self, bool enable);

#endif
