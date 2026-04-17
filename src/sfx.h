#ifndef SFX_H
#define SFX_H

#include <stdint.h>

void sfx_init(void);
void sfx_update(void);
void sfx_start(void);
void sfx_fire(void);
void sfx_hit(uint8_t frames);
void sfx_miss(void);

#endif
