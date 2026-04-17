#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "math.h"
#include "viewport.h"
#include "player.h"

void crosshair_init(void);
void crosshair_hide_view(void);
void crosshair_update_view(const player_t *player, const viewport_t *viewport);

#endif
