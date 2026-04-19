#include <gb/gb.h>

#include "game.h"
#include "viewport.h"
#include "crosshair.h"
#include "input.h"
#include "statusbar.h"
#include "sfx.h"

#define HIT_FRAMES  25
#define MISS_FRAMES 20
#define VIEWPORT_WORLD_Y 0

static vec2_t player_viewport_target(const player_t *player) {
    vec2_t target = vec2(player->pos.x + PLAYER_BODY_W / 2, VIEWPORT_WORLD_Y);
    return target;
}

static void reset_round(game_t *self) {
    DISPLAY_OFF;

    terrain_init(&self->terrain);
    viewport_init(&self->viewport,
                  vec2(0, 0),
                  vec2(self->terrain.size_px.x - SCREEN_W_PX,
                       self->terrain.size_px.y - SCREEN_H_PX));
    player_reset_round(&self->players[0], &self->terrain);
    player_reset_round(&self->players[1], &self->terrain);
    projectile_init(&self->projectile);

    self->active_player = 0;
    self->state_timer = 0;
    self->state = STATE_AIM;
    
    terrain_update_view(&self->terrain, &self->viewport);
    player_update_view(&self->players[0], &self->viewport);
    player_update_view(&self->players[1], &self->viewport);
    crosshair_update_view(&self->players[0], &self->viewport);
    statusbar_update_view(self->players);
    
    SHOW_BKG; SHOW_SPRITES; SHOW_WIN;
    DISPLAY_ON;

    sfx_start();
}

static void update_state_aim(game_t *self) {
    player_t *active_player = &self->players[self->active_player];

    if (input_pressed(J_A)) {
        crosshair_hide_view();
        projectile_launch(&self->projectile, active_player);
        sfx_fire();
        self->state = STATE_FIRE;
        return;
    }

    if (input_pressed(J_B)) {
        crosshair_hide_view();
        uint8_t target_idx = self->active_player ^ 1;
        viewport_pan_anim(&self->viewport, player_viewport_target(&self->players[target_idx]));
        self->state = STATE_PEEK;
        return;
    }

    vec2_t aim = vec2(0, 0);
    
    if (input_repeated(J_LEFT))       aim.x = -1;
    else if (input_repeated(J_RIGHT)) aim.x =  1;
    if (input_repeated(J_UP))         aim.y = -1;
    else if (input_repeated(J_DOWN))  aim.y =  1;

    if (aim.x || aim.y) {
        player_adjust_aim(active_player, aim);
        crosshair_update_view(active_player, &self->viewport);
    }
}

static void update_state_peek(game_t *self) {
    if (!input_held(J_B)) {
        viewport_pan_anim(&self->viewport, player_viewport_target(&self->players[self->active_player]));
    }

    if (input_pressed(J_B)) {
        uint8_t target_idx = self->active_player ^ 1;
        viewport_pan_anim(&self->viewport, player_viewport_target(&self->players[target_idx]));
    }

    bool panning = viewport_update_pan_anim(&self->viewport);
    if (panning) {
        terrain_update_view(&self->terrain, &self->viewport);
        player_update_view(&self->players[0], &self->viewport);
        player_update_view(&self->players[1], &self->viewport);
        return;
    }

    if (input_held(J_B)) return;

    self->state = STATE_AIM;
    crosshair_update_view(&self->players[self->active_player], &self->viewport);
}

static void update_state_fire(game_t *self) {
    uint8_t target_player = self->active_player ^ 1;
    player_t *target = &self->players[target_player];

    projectile_state_t projectile_state = projectile_simulate(&self->projectile, &self->terrain,target);
    projectile_update_view(&self->projectile, &self->viewport);

    switch (projectile_state) {
    case PROJECTILE_FLYING:
        vec2_t pan_target = vec2(FIXED_INT(self->projectile.pos.x), VIEWPORT_WORLD_Y);
        viewport_pan_immediate(&self->viewport, pan_target);
        terrain_update_view(&self->terrain, &self->viewport);
        player_update_view(&self->players[0], &self->viewport);
        player_update_view(&self->players[1], &self->viewport);
        break;
    case PROJECTILE_HIT_PLAYER:
        player_take_damage(target);
        player_set_shake(target, true);
        sfx_hit(HIT_FRAMES);
        statusbar_update_view(self->players);
        self->state_timer = HIT_FRAMES;
        self->state = STATE_HIT;
        break;
    case PROJECTILE_HIT_TERRAIN:
        vec2_t hit_pos = vec2(FIXED_INT(self->projectile.pos.x), FIXED_INT(self->projectile.pos.y));
        terrain_damage(&self->terrain, hit_pos);
        terrain_set_shake(&self->terrain, true);
        terrain_update_view(&self->terrain, &self->viewport);
        player_reset_position(&self->players[0], &self->terrain);
        player_update_view(&self->players[0], &self->viewport);
        player_reset_position(&self->players[1], &self->terrain);
        player_update_view(&self->players[1], &self->viewport);
        /* fallthrough */
    default:
        sfx_miss();
        self->state_timer = MISS_FRAMES;
        self->state = STATE_MISS;
    }
}

static void update_state_hit(game_t *self) {
    uint8_t target_player = self->active_player ^ 1;
    player_t *target = &self->players[target_player];

    sfx_update();
    player_update_view(target, &self->viewport);

    if (self->state_timer > 0) {
        self->state_timer--;
        return;
    }
    
    projectile_hide_view(&self->projectile);

    player_set_shake(target, false);
    player_update_view(target, &self->viewport);

    if (!target->lives) {
        self->players[self->active_player].score++;
        reset_round(self);
        return;
    }

    self->active_player ^= 1;
    self->state = STATE_PAN_CAM;
    viewport_pan_anim(&self->viewport, player_viewport_target(&self->players[self->active_player]));
}

static void update_state_miss(game_t *self) {
    terrain_update_view(&self->terrain, &self->viewport);

    if (self->state_timer > 0) {
        self->state_timer--;
        return;
    }

    projectile_hide_view(&self->projectile);

    terrain_set_shake(&self->terrain, false);
    terrain_update_view(&self->terrain, &self->viewport);

    self->active_player ^= 1;
    self->state = STATE_PAN_CAM;
    viewport_pan_anim(&self->viewport, player_viewport_target(&self->players[self->active_player]));
}

static void update_state_pan_cam(game_t *self) {
    bool panning = viewport_update_pan_anim(&self->viewport);
    if (panning) {
        terrain_update_view(&self->terrain, &self->viewport);
        player_update_view(&self->players[0], &self->viewport);
        player_update_view(&self->players[1], &self->viewport);
    } else {
        self->state = STATE_AIM;
        crosshair_update_view(&self->players[self->active_player], &self->viewport);
    }
}

void game_loop(void) {
    game_t game;

    player_init(&game.players[0], PLAYER_LEFT);
    player_init(&game.players[1], PLAYER_RIGHT);
    crosshair_init();
    statusbar_init();
    
    reset_round(&game);

    for (;;) {
        vsync();
        input_update();

        switch (game.state) {
        case STATE_AIM:     update_state_aim(&game);     break;
        case STATE_PEEK:    update_state_peek(&game);    break;
        case STATE_FIRE:    update_state_fire(&game);    break;
        case STATE_HIT:     update_state_hit(&game);     break;
        case STATE_MISS:    update_state_miss(&game);    break;
        case STATE_PAN_CAM: update_state_pan_cam(&game); break;
        }
    }
}
