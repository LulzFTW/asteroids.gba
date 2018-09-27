#include "myLib.h"
#include "sprites.h"
#include "trig.h"
#include "text.h"

#ifdef __thumb__
#define swi_call(x)   asm volatile("swi\t"#x ::: "r0", "r1", "r2", "r3")
#else
#define swi_call(x)   asm volatile("swi\t"#x"<<16" ::: "r0", "r1", "r2", "r3")
#endif

unsigned short *videoBuffer = (unsigned short *) 0x6000000;
charblock *charbase = ((charblock *) 0x6000000);
ObjAttr ARR_IN_IWRAM spriteBuffer[NUM_SPRITES];
ObjAffine *affineBuffer = (ObjAffine *) spriteBuffer;

void clear_screen()
{
    unsigned short black = 0;
    DMA[3].src = &black;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = WIDTH * HEIGHT | DMA_ON | DMA_SOURCE_FIXED;
}

void clear_rect(int x, int y, int w, int h)
{
    unsigned short black = 0;
    for (int i = 0; i < h; i++)
    {
        DMA[3].src = &black;
        DMA[3].dst = videoBuffer + ((i + y) * 240) + x;
        DMA[3].cnt = w | DMA_ON | DMA_SOURCE_FIXED;
    }
}

void drawCenteredString(int x, int y, char *str, int len)
{
    int xOffset = x - len * 3;
    clear_rect(xOffset, y, len * 6, 8);
    drawString(xOffset, y, str, WHITE);
}

void init_sprites()
{
    DMA[3].src = sprites_palette;
    DMA[3].dst = SPRITEPAL;
    DMA[3].cnt = SPRITES_PALETTE_SIZE | DMA_ON;

    DMA[3].src = sprites;
    DMA[3].dst = &charbase[5];
    DMA[3].cnt = SPRITES_SIZE | DMA_ON;

    for (int i = 0; i < NUM_SPRITES; i++)
        spriteBuffer[i].attr0 = ATTR0_HIDE;
}

void flush_sprites()
{
    DMA[3].src = spriteBuffer;
    DMA[3].dst = SPRITEMEM;
    DMA[3].cnt = (sizeof(spriteBuffer) / 2) | DMA_ON;
}

void obj_position(ObjAttr *attr, int x, int y)
{
    attr->attr0 = (attr->attr0 & ~ATTR0_Y) | (y & ATTR0_Y);
    attr->attr1 = (attr->attr1 & ~ATTR1_X) | (x & ATTR1_X);
}

void aff_rotation(ObjAffine *aff, int angle)
{
    int sin = sin(angle);
    int cos = cos(angle);
    aff_matrix(aff, cos, -sin, sin, cos);
}

void aff_matrix(ObjAffine *aff, int a, int b, int c, int d)
{
    aff->pa = a;
    aff->pb = b;
    aff->pc = c;
    aff->pd = d;
}

/*
 * because r, c is for scrubs
 */
void drawImage(int x, int y, int w, int h, const u16 *image)
{
    drawImage3(y, x, w, h, image);
}

void drawImage3(int r, int c, int w, int h, const u16 *image)
{
    for (int i = 0; i < h; i++)
    {
        DMA[3].src = image + i * w;
        DMA[3].dst = videoBuffer + ((i + r) * 240) + c;
        DMA[3].cnt = w | DMA_ON;
    }
}

void setPixel(int x, int y, unsigned short color)
{
	videoBuffer[x + y * 240] = color;
}

void waitForVblank()
{
    // VBlankIntrWait
    swi_call(0x05);
}
