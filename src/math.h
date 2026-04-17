#ifndef MATH_H
#define MATH_H

#include <stdint.h>

/* 24.8 fixed-point: 256 = 1.0 */
typedef int32_t fixed_t;

#define FIXED(x)        ((fixed_t)(x) << 8)
#define FIXED_INT(f)    ((int16_t)((f) >> 8))

static inline uint16_t clamp16(int16_t val, int16_t lo, int16_t hi) {
    if (val < lo) return (uint16_t)lo;
    if (val > hi) return (uint16_t)hi;
    return (uint16_t)val;
}

typedef struct {
    int16_t x;
    int16_t y;
} vec2_t;

static inline vec2_t vec2(int16_t x, int16_t y) {
    vec2_t v;

    v.x = x;
    v.y = y;

    return v;
}

typedef struct {
    fixed_t x;
    fixed_t y;
} vec2fixed_t;

static inline vec2fixed_t vec2fixed(fixed_t x, fixed_t y) {
    vec2fixed_t v;

    v.x = x;
    v.y = y;

    return v;
}

#endif
