#include <Lcd_dma.h>
#include "Picture.h"

Lcd_dma *lcd_dma;

void setup()
{
    int width = 320, height = 10;
    lcd_dma = new Lcd_dma(width, height);
    lcd_dma->SetBrightness(20);
    lcd_dma->fillScreen(0);
    uint16_t *framebuffer;
    for (int k = 0; k < 240 / height; ++k)
    {
        framebuffer = lcd_dma->GetFramebuffer();
        for (int i = 0; i < lcd_dma->GetHeight(); ++i)
            for (int j = 0; j < lcd_dma->GetWidth(); ++j)
                framebuffer[lcd_dma->GetWidth() * i + j] = picture[k * height + i][j];
        lcd_dma->Flip(0, k * height);
    }
}

void loop()
{
}