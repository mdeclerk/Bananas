#include <gb/gb.h>

#include "sfx.h"

#define HIT_RETRIGGER_FRAMES 8

static uint8_t hit_frames;
static uint8_t hit_ticks;

static void play_hit_noise(void) {
    NR41_REG = 0x10;
    NR42_REG = 0xf3;
    NR43_REG = 0x54;
    NR44_REG = 0xc0;
}

void sfx_init(void) {
    NR52_REG = AUDENA_ON;
    NR50_REG = AUDVOL_VOL_LEFT(7) | AUDVOL_VOL_RIGHT(7);
    NR51_REG = AUDTERM_1_LEFT | AUDTERM_1_RIGHT |
        AUDTERM_2_LEFT | AUDTERM_2_RIGHT |
        AUDTERM_4_LEFT | AUDTERM_4_RIGHT;
    hit_frames = 0;
    hit_ticks = 0;
}

void sfx_fire(void) {
    hit_frames = 0;
    NR10_REG = 0x16;
    NR11_REG = 0x80;
    NR12_REG = 0xf2;
    NR13_REG = 0x40;
    NR14_REG = 0xc3;
}

void sfx_hit(uint8_t frames) {
    hit_frames = frames;
    hit_ticks = 0;
    play_hit_noise();
}

void sfx_miss(void) {
    hit_frames = 0;
    NR21_REG = 0x80;
    NR22_REG = 0x73;
    NR23_REG = 0x80;
    NR24_REG = 0xc2;
}

void sfx_start(void) {
    hit_frames = 0;
    NR21_REG = 0x80;
    NR22_REG = 0x94;
    NR23_REG = 0xc0;
    NR24_REG = 0xc7;
}

void sfx_update(void) {
    if (hit_frames == 0) {
        return;
    }

    hit_frames--;
    hit_ticks++;

    if (hit_ticks >= HIT_RETRIGGER_FRAMES) {
        hit_ticks = 0;
        play_hit_noise();
    }
}
