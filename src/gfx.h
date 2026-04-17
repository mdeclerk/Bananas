#ifndef GFX_H
#define GFX_H

#include <stdint.h>

/*
 * VRAM tile layout — all loaded once by gfx_init().
 *
 * Sprite tiles (OAM, 0–15):
 *   0–8   Player body (kong, 3×3 grid)
 *   9–12  Projectile  (banana, 4 animation frames)
 *   13    Explosion
 *   14    Crosshair dot
 *   15    Crosshair mark
 *
 * BKG/WIN shared tile data (16–232):
 *   16–32  Font glyphs (0 1 2 3 4 5 6 7 8 9 P R E S T A <space>)
 *   33–34  HUD hearts  (full, empty)
 *   35–40  Terrain     (sky, surface, fill 0–3)
 *   41+    Splash image (title screen)
 */

/* Font glyphs — glyph order matches glyphs.png */
#define TILE_WIN_FONT_BASE  ((uint8_t)16u)
#define GLYPH_0      0
#define GLYPH_1      1
#define GLYPH_2      2
#define GLYPH_3      3
#define GLYPH_4      4
#define GLYPH_5      5
#define GLYPH_6      6
#define GLYPH_7      7
#define GLYPH_8      8
#define GLYPH_9      9
#define GLYPH_P      10
#define GLYPH_R      11
#define GLYPH_E      12
#define GLYPH_S      13
#define GLYPH_T      14
#define GLYPH_A      15
#define GLYPH_SPACE  16

/* HUD icons (immediately after font) */
#define TILE_WIN_HUD_BASE        ((uint8_t)33u)
#define TILE_WIN_HUD_HEART_FULL  TILE_WIN_HUD_BASE
#define TILE_WIN_HUD_HEART_EMPTY ((uint8_t)(TILE_WIN_HUD_BASE + 1u))

/* Terrain tiles (immediately after HUD) */
#define TILE_BKG_TERRAIN_BASE ((uint8_t)35u)
#define TILE_BKG_SKY          TILE_BKG_TERRAIN_BASE
#define TILE_BKG_SURFACE      ((uint8_t)(TILE_BKG_TERRAIN_BASE + 1u))
#define TILE_BKG_FILL0        ((uint8_t)(TILE_BKG_TERRAIN_BASE + 2u))
#define TILE_BKG_FILL1        ((uint8_t)(TILE_BKG_TERRAIN_BASE + 3u))
#define TILE_BKG_FILL2        ((uint8_t)(TILE_BKG_TERRAIN_BASE + 4u))
#define TILE_BKG_FILL3        ((uint8_t)(TILE_BKG_TERRAIN_BASE + 5u))

/* Splash image (immediately after terrain) */
#define TILE_BKG_SPLASH_BASE  ((uint8_t)41u)

/*
 * Sprite tiles (loaded into VRAM for sprites)
 *
 * Player body (24x24 = 9 tiles in 8x8 mode, 3x3 grid):
 *   tiles 0-8, row-major order
 *
 * Projectile:   tiles 9-12 (8x8 animation)
 * Explosion:    tile 13    (8x8)
 * Crosshair dot:  tile 14  (8x8)
 * Crosshair mark: tile 15  (8x8)
 *
 * Total sprite tiles: 16
 */
#define TILE_SPR_PLAYER_BASE        0
#define TILE_SPR_PROJECTILE_BASE    9
#define TILE_SPR_PROJECTILE_FRAMES  4
#define TILE_SPR_EXPLOSION          (TILE_SPR_PROJECTILE_BASE + TILE_SPR_PROJECTILE_FRAMES)
#define TILE_SPR_CROSSHAIR_DOT      (TILE_SPR_EXPLOSION + 1)
#define TILE_SPR_CROSSHAIR_MARK     (TILE_SPR_CROSSHAIR_DOT + 1)

void gfx_init(void);

void gfx_clear_bkg(void);
void gfx_print_bkg(uint8_t x, uint8_t y, const char *str);
void gfx_print_win(uint8_t x, uint8_t y, const char *str);

#endif
