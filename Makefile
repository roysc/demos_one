# build assets from art
TMP := $(shell mktemp -d)
TEXTURE_OUT := data/texture

all: garden garden-assets

garden-assets: sprites

garden:
	cmake --build build -j3 -t garden

sprites: \
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

.PHONY: garden sprites
