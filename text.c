#include "text.h"
#include "myLib.h"

void drawChar(int x, int y, char c, unsigned short color)
{
    const unsigned char *cdata = &fontdata_6x8[c * 48];
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            if (*cdata++)
            {
                setPixel(x + j, y + i, color);
            }
        }
    }
}

void drawString(int x, int y, char *str, unsigned short color)
{
	while (*str)
	{
		drawChar(x, y, *str++, color);
		x += 6;
	}
}
