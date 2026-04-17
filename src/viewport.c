#include <string.h>

#include "viewport.h"

#define VIEWPORT_STEP_MIN  1
#define VIEWPORT_STEP_MAX  12

void viewport_init(viewport_t *self, vec2_t min, vec2_t max) {
    self->min = min;
    self->max = max;
    self->pos = min;
    self->target = self->pos;
}

static vec2_t target_for_world(const viewport_t *self, vec2_t world) {
    vec2_t target = vec2(
        clamp16(world.x - ((int16_t)SCREEN_W_PX / 2), self->min.x, self->max.x),
        clamp16(world.y - ((int16_t)SCREEN_H_PX / 2), self->min.y, self->max.y)
    );
    return target;
}

void viewport_pan_immediate(viewport_t *self, vec2_t world) {
    self->pos = target_for_world(self, world);
    self->target = self->pos;
}

void viewport_pan_anim(viewport_t *self, vec2_t world) {
    self->target = target_for_world(self, world);
}

static int16_t update_axis(int16_t current, int16_t target) {
    int16_t delta = target - current;
    if (delta == 0) {
        return target;
    }

    int16_t abs_delta = delta > 0 ? delta : -delta;
    int16_t step = abs_delta >> 3;
    if (step < VIEWPORT_STEP_MIN) step = VIEWPORT_STEP_MIN;
    if (step > VIEWPORT_STEP_MAX) step = VIEWPORT_STEP_MAX;

    if (delta > 0) {
        return (delta <= step) ? target : current + step;
    }
    return (-delta <= step) ? target : current - step;
}

bool viewport_update_pan_anim(viewport_t *self) {
    if (self->pos.x == self->target.x && self->pos.y == self->target.y)
        return false;

    self->pos = vec2(
        update_axis(self->pos.x, self->target.x),
        update_axis(self->pos.y, self->target.y)
    );
    return true;
}

vec2_t viewport_to_screen(const viewport_t *self, vec2_t world) {
    vec2_t screen = vec2(world.x - self->pos.x, world.y - self->pos.y);
    return screen;
}
