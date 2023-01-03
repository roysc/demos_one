#!/usr/bin/make -f

TMP := $(shell mktemp -d)
TEXTURE_OUT := data/texture

all: garden garden-assets

garden:
	cmake --build build -t garden

# Build assets from art
garden-assets: \
	art/garden/sprites/*.ase \
	art/garden/tiles/*.ase \
	art/garden/walls/brick.ase \
	art/garden/others/*.ase

	aseprite_export.sh \
		-o $(TMP)/_sprites \
		$^
	pack_texture.py \
		-o $(TEXTURE_OUT)/garden \
		--tile-pivot 8,8 \
		$(TMP)/_sprites sprites

tags:
	ctags -Re src

.PHONY: garden garden-assets tags
