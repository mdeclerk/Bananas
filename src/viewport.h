#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <stdbool.h>
#include "math.h"

#define SCREEN_COLS   20
#define SCREEN_ROWS   18
#define SCREEN_W_PX   ((uint8_t)(SCREEN_COLS * 8))
#define SCREEN_H_PX   ((uint8_t)(SCREEN_ROWS * 8))

typedef struct {
    vec2_t pos;
    vec2_t target;
    vec2_t min;
    vec2_t max;
} viewport_t;

void    viewport_init(viewport_t *self, vec2_t min, vec2_t max);
void    viewport_pan_immediate(viewport_t *self, vec2_t world);
void    viewport_pan_anim(viewport_t *self, vec2_t world);
bool    viewport_update_pan_anim(viewport_t *self);
vec2_t  viewport_to_screen(const viewport_t *self, vec2_t world);

#endif
