#include <gb/gb.h>
#include "input.h"

#define INPUT_REPEAT_DELAY   8
#define INPUT_REPEAT_RATE    2
#define INPUT_REPEAT_MASK   (J_UP | J_DOWN | J_LEFT | J_RIGHT)

static uint8_t current_keys;
static uint8_t previous_keys;
static uint8_t repeat_keys;
static uint8_t repeat_ticks;

void input_init(void) {
    current_keys = 0;
    previous_keys = 0;
    repeat_keys = 0;
    repeat_ticks = 0;
}

void input_update(void) {
    previous_keys = current_keys;
    current_keys = joypad();

    uint8_t held_repeat_keys = current_keys & INPUT_REPEAT_MASK;
    if (held_repeat_keys != repeat_keys) {
        repeat_keys = held_repeat_keys;
        repeat_ticks = 0;
    } else if (repeat_ticks < 255) {
        repeat_ticks++;
    }
}

bool input_pressed(uint8_t mask) {
    return (current_keys & mask) && !(previous_keys & mask);
}

bool input_held(uint8_t mask) {
    return (current_keys & mask) != 0;
}

/* Fires on the initial press, then — while the key is held — waits
 * INPUT_REPEAT_DELAY frames and fires every INPUT_REPEAT_RATE frames after that. */
bool input_repeated(uint8_t mask) {
    if (input_pressed(mask)) {
        return true;
    }

    if (!(current_keys & mask)) {
        return false;
    }

    if (repeat_ticks < INPUT_REPEAT_DELAY) {
        return false;
    }

    return ((repeat_ticks - INPUT_REPEAT_DELAY) % INPUT_REPEAT_RATE) == 0;
}
