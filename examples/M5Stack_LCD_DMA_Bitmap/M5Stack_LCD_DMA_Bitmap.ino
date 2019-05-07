#include <Lcd_dma.h>
#include "Picture.h"

Lcd_dma *lcd_dma;

uint16_t changeEndian(uint16_t p)
{
    return (p >> 8) | (p << 8);
}

void setup()
{
    const int height = 10;
    lcd_dma = new Lcd_dma(320, height);
    lcd_dma->SetBrightness(20);
    lcd_dma->fillScreen(0);
    for (int k = 0; k < 240 / height; ++k)
    {
        uint16_t *framebuffer = lcd_dma->GetFramebuffer();
        for (int i = 0; i < lcd_dma->GetHeight(); ++i)
            for (int j = 0; j < lcd_dma->GetWidth(); ++j)
                framebuffer[lcd_dma->GetWidth() * i + j] = changeEndian(picture[240 - 1 - k * height - i][j]);
        lcd_dma->Flip(0, k * height);
    }
}

void loop()
{
}