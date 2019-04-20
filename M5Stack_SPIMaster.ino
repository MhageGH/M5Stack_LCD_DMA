#include "Lcd_dma.h"

Lcd_dma *lcd_dma;

void ReviseFramebuffer(uint16_t *framebuffer)
{
    for (int i = 0; i < lcd_dma->GetHeight(); ++i)
    {
        for (int j = 0; j < lcd_dma->GetWidth(); ++j)
        {
            uint16_t r = (uint16_t)(31.0f * (float)(j) / 200.0f);
            uint16_t g = (uint16_t)(63.0f * (float)(j) / 200.0f);
            uint16_t b = (uint16_t)(31.0f * (float)(j) / 200.0f);
            uint16_t c = (r << 11) | (g << 5) | (b << 0);
            c = (c << 8) | (c >> 8);
            framebuffer[lcd_dma->GetWidth() * i + j] = c;
        }
    }
}

void setup()
{
    lcd_dma = new Lcd_dma(200, 150);
    lcd_dma->fillScreen(0);
}

void loop()
{
    static uint16_t *framebuffer = lcd_dma->GetFramebuffer();
    ReviseFramebuffer(framebuffer);
    delay(60);
    lcd_dma->Flip(60, 45);
    framebuffer = lcd_dma->GetFramebuffer();
}