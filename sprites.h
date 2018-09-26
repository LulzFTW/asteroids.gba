/*
 * Exported with nin10kit v1.4
 * Invocation command was nin10kit -mode=sprites -bpp=4 -for_bitmap=1 -transparent=ff00ff sprites ship0.png ship1.png ship2.png bullet.png asteroid1.png asteroid2.png asteroid3.png 
 * Time-stamp: Sunday 04/02/2017, 21:43:01
 * 
 * Image Information
 * -----------------
 * ship0.png 8@16
 * ship1.png 8@16
 * ship2.png 8@16
 * bullet.png 8@8
 * asteroid1.png 32@32
 * asteroid2.png 16@16
 * asteroid3.png 8@8
 * Transparent color: (255, 0, 255)
 * 
 * Quote/Fortune of the Day!
 * -------------------------
 * 
 * All bug reports / feature requests are to be sent to Brandon (bwhitehead0308@gmail.com)
 */

#ifndef SPRITES_H
#define SPRITES_H

#define SPRITES_TRANSPARENT 0x00

#define SPRITES_PALETTE_TYPE (0 << 13)
#define SPRITES_DIMENSION_TYPE (1 << 6)

extern const unsigned short sprites_palette[256];
#define SPRITES_PALETTE_SIZE 256

extern const unsigned short sprites[448];
#define SPRITES_SIZE 448

#define SHIP1_PALETTE (0 << 12)
#define SHIP1_SPRITE_SHAPE (2 << 14)
#define SHIP1_SPRITE_SIZE (0 << 14)
#define SHIP1_ID 512

#define SHIP0_PALETTE (0 << 12)
#define SHIP0_SPRITE_SHAPE (2 << 14)
#define SHIP0_SPRITE_SIZE (0 << 14)
#define SHIP0_ID 514

#define SHIP2_PALETTE (0 << 12)
#define SHIP2_SPRITE_SHAPE (2 << 14)
#define SHIP2_SPRITE_SIZE (0 << 14)
#define SHIP2_ID 516

#define BULLET_PALETTE (0 << 12)
#define BULLET_SPRITE_SHAPE (0 << 14)
#define BULLET_SPRITE_SIZE (0 << 14)
#define BULLET_ID 518

#define ASTEROID1_PALETTE (0 << 12)
#define ASTEROID1_SPRITE_SHAPE (0 << 14)
#define ASTEROID1_SPRITE_SIZE (2 << 14)
#define ASTEROID1_ID 519

#define ASTEROID2_PALETTE (0 << 12)
#define ASTEROID2_SPRITE_SHAPE (0 << 14)
#define ASTEROID2_SPRITE_SIZE (1 << 14)
#define ASTEROID2_ID 535

#define ASTEROID3_PALETTE (0 << 12)
#define ASTEROID3_SPRITE_SHAPE (0 << 14)
#define ASTEROID3_SPRITE_SIZE (0 << 14)
#define ASTEROID3_ID 539

#endif

