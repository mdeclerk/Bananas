#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "player.h"

void statusbar_init(void);
void statusbar_update_view(const player_t *players);

#endif
