/*
 * $Id$
 */

#include "vc6.h" // Fixes things if you're using VC6, does nothing if otherwise

#include "debug.h"
#include "dungeonview.h"
#include "image.h"
#include "imagemgr.h"
#include "settings.h"
#include "screen.h"
#include "tileanim.h"
#include "tileset.h"
#include "u4.h"
#include "error.h"


DungeonView::DungeonView(int x, int y, int columns, int rows) : TileView(x, y, rows, columns)
, screen3dDungeonViewEnabled(true)
{
}


DungeonView * DungeonView::instance(NULL);
DungeonView * DungeonView::getInstance()
{
	if (!instance) 	{
		instance = new DungeonView(BORDER_WIDTH, BORDER_HEIGHT, VIEWPORT_W, VIEWPORT_H);
	}
	return instance;
}

void DungeonView::display(Context * c, TileView *view)
{
	int x,y;

    /* 1st-person perspective */
    if (screen3dDungeonViewEnabled) {
        //Note: This shouldn't go above 4, unless we check opaque tiles each step of the way.
        const int farthest_non_wall_tile_visibility = 4;

    	vector<MapTile> tiles;

        screenEraseMapArea();
        if (c->party->getTorchDuration() > 0) {
            for (y = 3; y >= 0; y--) {
                DungeonGraphicType type;

                //FIXME: Maybe this should be in a loop
				tiles = getTiles(y, -1);
                type = tilesToGraphic(tiles);
				drawWall(-1, y, (Direction)c->saveGame->orientation, type);

				tiles = getTiles(y, 1);
                type = tilesToGraphic(tiles);
                drawWall(1, y, (Direction)c->saveGame->orientation, type);

                tiles = getTiles(y, 0);
                type = tilesToGraphic(tiles);
                drawWall(0, y, (Direction)c->saveGame->orientation, type);

                //This only checks that the tile at y==3 is opaque
                if (y == 3 && !tiles.front().getTileType()->isOpaque())
               	{
               		for (int y_obj = farthest_non_wall_tile_visibility; y_obj > y; y_obj--)
               		{
               		vector<MapTile> distant_tiles = getTiles(y_obj     , 0);
               		DungeonGraphicType distant_type = tilesToGraphic(distant_tiles);

					if ((distant_type == DNGGRAPHIC_DNGTILE) || (distant_type == DNGGRAPHIC_BASETILE))
						drawTile(c->location->map->tileset->get(distant_tiles.front().getId()),0, y_obj, Direction(c->saveGame->orientation));
               		}
               	}
				if ((type == DNGGRAPHIC_DNGTILE) || (type == DNGGRAPHIC_BASETILE))
					drawTile(c->location->map->tileset->get(tiles.front().getId()), 0, y, Direction(c->saveGame->orientation));
            }
        }
    }

    /* 3rd-person perspective */
    else {
    	vector<MapTile> tiles;

        static MapTile black = c->location->map->tileset->getByName("black")->getId();
        static MapTile avatar = c->location->map->tileset->getByName("avatar")->getId();

        for (y = 0; y < VIEWPORT_H; y++) {
            for (x = 0; x < VIEWPORT_W; x++) {
                    tiles = getTiles((VIEWPORT_H / 2) - y, x - (VIEWPORT_W / 2));

				/* Only show blackness if there is no light */
				if (c->party->getTorchDuration() <= 0)
					view->drawTile(black, false, x, y);
				else if (x == VIEWPORT_W/2 && y == VIEWPORT_H/2)
					view->drawTile(avatar, false, x, y);
				else
					view->drawTile(tiles, false, x, y);
            }
        }
    }
}

void DungeonView::drawInDungeon(Tile *tile, int x_offset, int distance, Direction orientation, bool tiledWall) {
	Image *scaled;

  	const static int nscale_vga[] = { 12, 8, 4, 2, 1};
    const static int nscale_ega[] = { 8, 4, 2, 1, 0};

	const int lscale_vga[] = { 22, 18, 10, 4, 1};
	const int lscale_ega[] = { 22, 14, 6, 3, 1};

    const int * lscale;
    const int * nscale;
    int offset_multiplier = 0;
    int offset_adj = 0;
    if (settings.videoType != "EGA")
    {
    	lscale = & lscale_vga[0];
    	nscale = & nscale_vga[0];
    	offset_multiplier = 1;
    	offset_adj = 2;
    }
    else
    {
    	lscale = & lscale_ega[0];
    	nscale = & nscale_ega[0];
    	offset_adj = 1;
    	offset_multiplier = 4;
    }

    const int *dscale = tiledWall ? lscale : nscale;

    //Clear scratchpad and set a background color
    animated->initializeToBackgroundColor();
    //Put tile on animated scratchpad
    if (tile->getAnim()) {
        MapTile mt = tile->getId();
        tile->getAnim()->draw(animated, tile, mt, orientation);
    }
    else
    {
        tile->getImage()->drawOn(animated, 0, 0);
    }
    animated->makeBackgroundColorTransparent();
    //This process involving the background color is only required for drawing in the dungeon.
    //It will not play well with semi-transparent graphics.

    /* scale is based on distance; 1 means half size, 2 regular, 4 means scale by 2x, etc. */
    if (dscale[distance] == 0)
		return;
    else if (dscale[distance] == 1)
        scaled = screenScaleDown(animated, 2);
    else
    {
        scaled = screenScale(animated, dscale[distance] / 2, 1, 0);
    }

    if (tiledWall) {
    	int i_x = SCALED((VIEWPORT_W * tileWidth  / 2.0) + this->x) - (scaled->width() / 2.0);
    	int i_y = SCALED((VIEWPORT_H * tileHeight / 2.0) + this->y) - (scaled->height() / 2.0);
    	int f_x = i_x + scaled->width();
    	int f_y = i_y + scaled->height();
    	int d_x = animated->width();
    	int d_y = animated->height();

    	for (int x = i_x; x < f_x; x+=d_x)
    		for (int y = i_y; y < f_y; y+=d_y)
    			animated->drawSubRectOn(this->screen,
    					x,
    					y,
    					0,
    					0,
    					f_x - x,
    					f_y - y
    			);
    }
    else {
    	int y_offset = std::max(0,(dscale[distance] - offset_adj) * offset_multiplier);
    	int x = SCALED((VIEWPORT_W * tileWidth / 2.0) + this->x) - (scaled->width() / 2.0);
    	int y = SCALED((VIEWPORT_H * tileHeight / 2.0) + this->y + y_offset) - (scaled->height() / 8.0);

		scaled->drawSubRectOn(	this->screen,
								x,
								y,
								0,
								0,
								SCALED(tileWidth * VIEWPORT_W + this->x) - x ,
								SCALED(tileHeight * VIEWPORT_H + this->y) - y );
    }

    delete scaled;
}

int DungeonView::graphicIndex(int xoffset, int distance, Direction orientation, DungeonGraphicType type) {
    int index;

    index = 0;

    if (type == DNGGRAPHIC_LADDERUP && xoffset == 0)
        return 48 +
        (distance * 2) +
        (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH) ? 1 : 0);

    if (type == DNGGRAPHIC_LADDERDOWN && xoffset == 0)
        return 56 +
        (distance * 2) +
        (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH) ? 1 : 0);

    if (type == DNGGRAPHIC_LADDERUPDOWN && xoffset == 0)
        return 64 +
        (distance * 2) +
        (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH) ? 1 : 0);

    /* FIXME */
    if (type != DNGGRAPHIC_WALL && type != DNGGRAPHIC_DOOR)
        return -1;

    if (type == DNGGRAPHIC_DOOR)
        index += 24;

    index += (xoffset + 1) * 2;

    index += distance * 6;

    if (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH))
        index++;

    return index;
}

void DungeonView::drawTile(Tile *tile, int x_offset, int distance, Direction orientation) {
    // Draw the tile to the screen
	DungeonViewer.drawInDungeon(tile, x_offset, distance, orientation, tile->isTiledInDungeon());
}

std::vector<MapTile> DungeonView::getTiles(int fwd, int side) {
    MapCoords coords = c->location->coords;

    switch (c->saveGame->orientation) {
    case DIR_WEST:
        coords.x -= fwd;
        coords.y -= side;
        break;

    case DIR_NORTH:
        coords.x += side;
        coords.y -= fwd;
        break;

    case DIR_EAST:
        coords.x += fwd;
        coords.y += side;
        break;

    case DIR_SOUTH:
        coords.x -= side;
        coords.y += fwd;
        break;

    case DIR_ADVANCE:
    case DIR_RETREAT:
    default:
        ASSERT(0, "Invalid dungeon orientation");
    }

    // Wrap the coordinates if necessary
    coords.wrap(c->location->map);

    bool focus;
    return c->location->tilesAt(coords, focus);
}

DungeonGraphicType DungeonView::tilesToGraphic(const std::vector<MapTile> &tiles) {
    MapTile tile = tiles.front();

    static const MapTile corridor = c->location->map->tileset->getByName("brick_floor")->getId();
    static const MapTile up_ladder = c->location->map->tileset->getByName("up_ladder")->getId();
    static const MapTile down_ladder = c->location->map->tileset->getByName("down_ladder")->getId();
    static const MapTile updown_ladder = c->location->map->tileset->getByName("up_down_ladder")->getId();

    /*
     * check if the dungeon tile has an annotation or object on top
     * (always displayed as a tile, unless a ladder)
     */
    if (tiles.size() > 1) {
        if (tile.id == up_ladder.id)
            return DNGGRAPHIC_LADDERUP;
        else if (tile.id == down_ladder.id)
            return DNGGRAPHIC_LADDERDOWN;
        else if (tile.id == updown_ladder.id)
            return DNGGRAPHIC_LADDERUPDOWN;
        else if (tile.id == corridor.id)
            return DNGGRAPHIC_NONE;
        else
            return DNGGRAPHIC_BASETILE;
    }

    /*
     * if not an annotation or object, then the tile is a dungeon
     * token
     */
    Dungeon *dungeon = dynamic_cast<Dungeon *>(c->location->map);
    DungeonToken token = dungeon->tokenForTile(tile);

    switch (token) {
    case DUNGEON_TRAP:
    case DUNGEON_CORRIDOR:
        return DNGGRAPHIC_NONE;
    case DUNGEON_WALL:
    case DUNGEON_SECRET_DOOR:
        return DNGGRAPHIC_WALL;
    case DUNGEON_ROOM:
    case DUNGEON_DOOR:
        return DNGGRAPHIC_DOOR;
    case DUNGEON_LADDER_UP:
        return DNGGRAPHIC_LADDERUP;
    case DUNGEON_LADDER_DOWN:
        return DNGGRAPHIC_LADDERDOWN;
    case DUNGEON_LADDER_UPDOWN:
        return DNGGRAPHIC_LADDERUPDOWN;

    default:
        return DNGGRAPHIC_DNGTILE;
    }
}

const struct {
    const char *subimage;
    int ega_x2, ega_y2;
    int vga_x2, vga_y2;
    const char *subimage2;
} dngGraphicInfo[] = {
    { "dung0_lft_ew" },
    { "dung0_lft_ns" },
    { "dung0_mid_ew" },
    { "dung0_mid_ns" },
    { "dung0_rgt_ew" },
    { "dung0_rgt_ns" },

    { "dung1_lft_ew", 0, 32, 0, 8, "dung1_xxx_ew" },
    { "dung1_lft_ns", 0, 32, 0, 8, "dung1_xxx_ns" },
    { "dung1_mid_ew" },
    { "dung1_mid_ns" },
    { "dung1_rgt_ew", 144, 32, 160, 8, "dung1_xxx_ew" },
    { "dung1_rgt_ns", 144, 32, 160, 8, "dung1_xxx_ns" },

    { "dung2_lft_ew", 0, 64, 0, 48, "dung2_xxx_ew" },
    { "dung2_lft_ns", 0, 64, 0, 48, "dung2_xxx_ns" },
    { "dung2_mid_ew" },
    { "dung2_mid_ns" },
    { "dung2_rgt_ew", 112, 64, 128, 48, "dung2_xxx_ew" },
    { "dung2_rgt_ns", 112, 64, 128, 48, "dung2_xxx_ns" },

    { "dung3_lft_ew", 0, 80, 48, 72, "dung3_xxx_ew" },
    { "dung3_lft_ns", 0, 80, 48, 72, "dung3_xxx_ns" },
    { "dung3_mid_ew" },
    { "dung3_mid_ns" },
    { "dung3_rgt_ew", 96, 80, 104, 72, "dung3_xxx_ew" },
    { "dung3_rgt_ns", 96, 80, 104, 72, "dung3_xxx_ns" },

    { "dung0_lft_ew_door" },
    { "dung0_lft_ns_door" },
    { "dung0_mid_ew_door" },
    { "dung0_mid_ns_door" },
    { "dung0_rgt_ew_door" },
    { "dung0_rgt_ns_door" },

    { "dung1_lft_ew_door", 0, 32, 0, 8, "dung1_xxx_ew" },
    { "dung1_lft_ns_door", 0, 32, 0, 8, "dung1_xxx_ns" },
    { "dung1_mid_ew_door" },
    { "dung1_mid_ns_door" },
    { "dung1_rgt_ew_door", 144, 32, 160, 8, "dung1_xxx_ew" },
    { "dung1_rgt_ns_door", 144, 32, 160, 8, "dung1_xxx_ns" },

    { "dung2_lft_ew_door", 0, 64, 0, 48, "dung2_xxx_ew" },
    { "dung2_lft_ns_door", 0, 64, 0, 48, "dung2_xxx_ns" },
    { "dung2_mid_ew_door" },
    { "dung2_mid_ns_door" },
    { "dung2_rgt_ew_door", 112, 64, 128, 48, "dung2_xxx_ew" },
    { "dung2_rgt_ns_door", 112, 64, 128, 48, "dung2_xxx_ns" },

    { "dung3_lft_ew_door", 0, 80, 48, 72, "dung3_xxx_ew" },
    { "dung3_lft_ns_door", 0, 80, 48, 72, "dung3_xxx_ns" },
    { "dung3_mid_ew_door" },
    { "dung3_mid_ns_door" },
    { "dung3_rgt_ew_door", 96, 80, 104, 72, "dung3_xxx_ew" },
    { "dung3_rgt_ns_door", 96, 80, 104, 72, "dung3_xxx_ns" },

    { "dung0_ladderup" },
    { "dung0_ladderup_side" },
    { "dung1_ladderup" },
    { "dung1_ladderup_side" },
    { "dung2_ladderup" },
    { "dung2_ladderup_side" },
    { "dung3_ladderup" },
    { "dung3_ladderup_side" },

    { "dung0_ladderdown" },
    { "dung0_ladderdown_side" },
    { "dung1_ladderdown" },
    { "dung1_ladderdown_side" },
    { "dung2_ladderdown" },
    { "dung2_ladderdown_side" },
    { "dung3_ladderdown" },
    { "dung3_ladderdown_side" },

    { "dung0_ladderupdown" },
    { "dung0_ladderupdown_side" },
    { "dung1_ladderupdown" },
    { "dung1_ladderupdown_side" },
    { "dung2_ladderupdown" },
    { "dung2_ladderupdown_side" },
    { "dung3_ladderupdown" },
    { "dung3_ladderupdown_side" },
};

void DungeonView::drawWall(int xoffset, int distance, Direction orientation, DungeonGraphicType type) {
    int index;

    index = graphicIndex(xoffset, distance, orientation, type);
    if (index == -1 || distance >= 4)
        return;

    int x = 0, y = 0;
    SubImage *subimage = imageMgr->getSubImage(dngGraphicInfo[index].subimage);
    if (subimage) {
        x = subimage->x;
        y = subimage->y;
    }

    screenDrawImage(dngGraphicInfo[index].subimage, (BORDER_WIDTH + x) * settings.scale,
                    (BORDER_HEIGHT + y) * settings.scale);

    if (dngGraphicInfo[index].subimage2 != NULL) {
        // FIXME: subimage2 is a horrible hack, needs to be cleaned up
        if (settings.videoType == "EGA")
            screenDrawImage(dngGraphicInfo[index].subimage2,
                            (8 + dngGraphicInfo[index].ega_x2) * settings.scale,
                            (8 + dngGraphicInfo[index].ega_y2) * settings.scale);
        else
            screenDrawImage(dngGraphicInfo[index].subimage2,
                            (8 + dngGraphicInfo[index].vga_x2) * settings.scale,
                            (8 + dngGraphicInfo[index].vga_y2) * settings.scale);
    }
}

