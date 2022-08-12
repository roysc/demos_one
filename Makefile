# build assets from art

all: walls 
walls:
	aseprite_export.sh \
		-o _walls art/garden/walls/brick.ase
	pack_texture.py \
		-o data/texture/garden/ \
		--tile-pivot 8,8 \
		_walls walls

