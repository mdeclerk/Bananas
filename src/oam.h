#ifndef OAM_H
#define OAM_H

#include <stdint.h>

#include "viewport.h"

/*
 * Centralized OAM sprite slot allocation and helpers.
 *
 * Game Boy has 40 OAM entries (0–39).  Every module that owns
 * sprites reserves a contiguous range here so slot conflicts
 * are caught in one place.
 *
 *   Slot   Owner
 *   ----   -----
 *    0– 8  Player 0  (3×3 body = 9 sprites)
 *    9–17  Player 1  (3×3 body = 9 sprites)
 *   18     Projectile
 *   19–23  Crosshair (5 dots/marks)
 */

#define OAM_PLAYER0          0
#define OAM_PLAYER1          9
#define OAM_PROJECTILE      18
#define OAM_CROSSHAIR_FIRST 19
#define OAM_CROSSHAIR_COUNT  5

/* Total OAM entries currently in use (update when adding sprites). */
#define OAM_TOTAL_USED      24

/* DMG OAM has an 8px X offset and 16px Y offset — screen (0,0) is at OAM (8,16). */
#define OAM_X_OFS    8
#define OAM_Y_OFS   16
#define SPRITE_PX    8
#define SPRITE_HALF  (SPRITE_PX / 2)

void oam_hide(uint8_t first_id, uint8_t count);
void oam_move_to(uint8_t sprite_id, vec2_t world, const viewport_t *viewport);

#endif
