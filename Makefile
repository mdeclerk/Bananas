GBDK_HOME ?= /opt/gbdk

BUILD_DIR := build
ROM := $(BUILD_DIR)/bananas.gb
GEN_DIR := $(BUILD_DIR)/generated
SRC_DIR := src
ASSET_DIR := assets

CC := $(GBDK_HOME)/bin/lcc
PNG2ASSET := $(GBDK_HOME)/bin/png2asset

ASSET_PNGS := $(wildcard $(ASSET_DIR)/*.png)
ASSET_SRCS := $(patsubst $(ASSET_DIR)/%.png,$(GEN_DIR)/%.c,$(ASSET_PNGS))
ASSET_HDRS := $(ASSET_SRCS:.c=.h)
SRCS := $(shell find $(SRC_DIR) -name '*.c') $(ASSET_SRCS)
HDRS := $(shell find $(SRC_DIR) -name '*.h') $(ASSET_HDRS)

CPPFLAGS := -I$(BUILD_DIR)

ifeq ($(DEBUG),1)
CFLAGS += -Wa-l -Wl-j -Wl-m -Wm-yS
endif

PNG2ASSET_FLAGS := -keep_duplicate_tiles -no_palettes -map -bpp 2 -tiles_only -pack_mode gb -noflip -keep_palette_order

all: $(ROM)

$(BUILD_DIR) $(GEN_DIR):
	mkdir -p $@

$(GEN_DIR)/%.c: $(ASSET_DIR)/%.png Makefile | $(GEN_DIR)
	$(PNG2ASSET) $< $(PNG2ASSET_FLAGS) -c $@

# png2asset writes the matching header as a side effect of generating the C file.
$(GEN_DIR)/%.h: $(GEN_DIR)/%.c ;

$(ROM): $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
