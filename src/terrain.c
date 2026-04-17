#include <gb/gb.h>
#include <rand.h>
#include "terrain.h"
#include "gfx.h"

#define TERRAIN_MIN_HEIGHT 4
#define TERRAIN_MAX_HEIGHT 12
#define TERRAIN_HEIGHT_RANGE (TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT + 1)
#define TERRAIN_MAX_HEIGHT_DELTA 4
#define BG_MAP_COLS 32
#define TERRAIN_VIEW_COLS (SCREEN_COLS + 1)
#define SHAKE_OFFSET 2

void terrain_init(terrain_t *self) {
    self->loaded_view_col = 0;
    self->shake = false;
    terrain_randomize(self);
}

static const uint8_t terrain_sizes[] = {
    TERRAIN_SIZE_SMALL, TERRAIN_SIZE_MEDIUM, TERRAIN_SIZE_LARGE
};

void terrain_set_shake(terrain_t *self, bool enable) {
    self->shake = enable;
}

static uint8_t tile_at(const terrain_t *self, uint16_t world_col, uint8_t row) {
    if (world_col >= ((uint16_t)self->plateau_count * PLATEAU_WIDTH)) {
        return TILE_BKG_SKY;
    }

    uint8_t p = (uint8_t)(world_col / PLATEAU_WIDTH);
    if (p >= self->plateau_count) {
        return TILE_BKG_SKY;
    }

    uint8_t surface_row = SCREEN_ROWS - self->heights[p];
    if (row == surface_row) {
        return TILE_BKG_SURFACE;
    }
    if (row > surface_row) {
        return TILE_BKG_FILL0 + (p & 3u);
    }

    return TILE_BKG_SKY;
}

static void update_view_full(terrain_t *self, uint16_t first_world_col) {
    uint8_t dst_col = (uint8_t)(first_world_col & (BG_MAP_COLS - 1));
    
    uint8_t first_chunk = BG_MAP_COLS - dst_col;
    if (first_chunk > TERRAIN_VIEW_COLS) {
        first_chunk = TERRAIN_VIEW_COLS;
    }
    uint8_t second_chunk = TERRAIN_VIEW_COLS - first_chunk;

    for (uint8_t r = 0; r < SCREEN_ROWS; r++) {
        uint8_t row_buf[TERRAIN_VIEW_COLS];
        for (uint8_t c = 0; c < TERRAIN_VIEW_COLS; c++) {
            row_buf[c] = tile_at(self, first_world_col + c, r);
        }

        set_bkg_tiles(dst_col, r, first_chunk, 1, row_buf);
        if (second_chunk > 0) {
            set_bkg_tiles(0, r, second_chunk, 1, row_buf + first_chunk);
        }
    }

    self->loaded_view_col = first_world_col;
}

static void update_view_column(const terrain_t *self, uint16_t world_col) {
    uint8_t col_buf[SCREEN_ROWS];
    for (uint8_t r = 0; r < SCREEN_ROWS; r++) {
        col_buf[r] = tile_at(self, world_col, r);
    }
    
    uint8_t dst_col = (uint8_t)(world_col & (BG_MAP_COLS - 1));
    set_bkg_tiles(dst_col, 0, 1, SCREEN_ROWS, col_buf);
}

void terrain_update_view(terrain_t *self, const viewport_t *viewport) {
    int8_t shake_offset = 0;
    if (self->shake) {
        shake_offset = (sys_time & 1u) ? SHAKE_OFFSET : -SHAKE_OFFSET;
    }

    uint16_t scroll_x = clamp16(viewport->pos.x + shake_offset, 0, viewport->max.x);
    uint16_t scroll_y = clamp16(viewport->pos.y, 0, viewport->max.y);
    uint16_t first_world_col = scroll_x >> 3;
    int16_t delta = (int16_t)first_world_col - (int16_t)self->loaded_view_col;

    if (delta >= TERRAIN_VIEW_COLS || delta <= -TERRAIN_VIEW_COLS) {
        update_view_full(self, first_world_col);
    } else if (delta > 0) {
        for (uint8_t c = 0; c < (uint8_t)delta; c++) {
            update_view_column(self, self->loaded_view_col + TERRAIN_VIEW_COLS + c);
        }
    } else if (delta < 0) {
        for (uint8_t c = 0; c < (uint8_t)-delta; c++) {
            update_view_column(self, first_world_col + c);
        }
    }

    self->loaded_view_col = first_world_col;
    move_bkg((uint8_t)scroll_x, (uint8_t)scroll_y);
}

static uint8_t pos_to_plateau(int16_t x, uint8_t plateau_count) {
    if (x < 0) 
        return plateau_count;

    uint8_t p = (uint8_t)((uint16_t)x / 8 / PLATEAU_WIDTH);
    return (p >= plateau_count) ? plateau_count : p;
}

bool terrain_hit_test(const terrain_t *self, vec2_t pos) {
    uint8_t plateau = pos_to_plateau(pos.x, self->plateau_count);

    if (plateau >= self->plateau_count || pos.y < 0) {
        return false;
    }

    return pos.y >= (int16_t)(SCREEN_ROWS - self->heights[plateau]) * 8;
}

void terrain_damage(terrain_t *self, vec2_t hit_pos) {
    uint8_t plateau = pos_to_plateau(hit_pos.x, self->plateau_count);

    if (plateau >= self->plateau_count) return;
    if (self->heights[plateau] <= TERRAIN_MIN_HEIGHT) return;

    self->heights[plateau]--;

    uint16_t first_col = (uint16_t)plateau * PLATEAU_WIDTH;
    uint16_t view_end = self->loaded_view_col + TERRAIN_VIEW_COLS;

    for (uint8_t c = 0; c < PLATEAU_WIDTH; c++) {
        uint16_t col = first_col + c;
        if (col >= self->loaded_view_col && col < view_end) {
            update_view_column(self, col);
        }
    }
}

void terrain_randomize(terrain_t *self) {
    self->plateau_count = terrain_sizes[rand() % 3];
    for (uint8_t i = 0; i < self->plateau_count; i++) {
        self->heights[i] = TERRAIN_MIN_HEIGHT + (rand() % TERRAIN_HEIGHT_RANGE);
    }

    self->heights[1] = self->heights[0];
    self->heights[self->plateau_count - 2] = self->heights[self->plateau_count - 1];

    self->size_px = vec2(
        (uint16_t)self->plateau_count * PLATEAU_WIDTH * 8,
        SCREEN_H_PX
    );

    update_view_full(self, 0);
}
