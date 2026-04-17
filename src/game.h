#ifndef GAME_H
#define GAME_H

#include "viewport.h"
#include "crosshair.h"
#include "player.h"
#include "projectile.h"
#include "terrain.h"

/*
 * Game state machine:
 *
 *   AIM ──A──► FIRE ──► HIT  ──► PAN_CAM ──► AIM   (swap turns if target still alive;
 *    │         │                                    reset round + terrain if lives=0)
 *    │         └─────► MISS ──► PAN_CAM ──► AIM   (swap turns)
 *    │
 *    └──B──► PEEK ──B-release──► AIM               (temporary camera pan to opponent)
 */
typedef enum {
    STATE_AIM,
    STATE_PEEK,
    STATE_FIRE,
    STATE_HIT,
    STATE_MISS,
    STATE_PAN_CAM
} game_state_t;

typedef struct {
    viewport_t viewport;
    terrain_t terrain;
    player_t players[2];
    projectile_t projectile;
    game_state_t state;
    uint8_t active_player;
    uint8_t state_timer;
} game_t;

void game_loop(void);

#endif
