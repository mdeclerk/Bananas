#ifndef TERRAIN_H
#define TERRAIN_H

#include <stdbool.h>
#include <stdint.h>
#include "viewport.h"

#define PLATEAU_MAX_COUNT       24
#define PLATEAU_WIDTH            3   /* tile columns per plateau */
#define TERRAIN_SIZE_SMALL      12
#define TERRAIN_SIZE_MEDIUM     18
#define TERRAIN_SIZE_LARGE      24
#define TERRAIN_MAX_COLS        (PLATEAU_MAX_COUNT * PLATEAU_WIDTH)

typedef struct {
    uint8_t heights[PLATEAU_MAX_COUNT];
    uint8_t plateau_count;
    vec2_t size_px;
    bool shake;
    uint16_t loaded_view_col;
} terrain_t;

void terrain_init(terrain_t *self);
void terrain_randomize(terrain_t *self);
void terrain_set_shake(terrain_t *self, bool enable);
void terrain_update_view(terrain_t *self, const viewport_t *viewport);
bool terrain_hit_test(const terrain_t *self, vec2_t pos);
void terrain_damage(terrain_t *self, vec2_t hit_pos);

#endif
