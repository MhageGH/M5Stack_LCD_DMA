#include "Lcd_dma.h"

Lcd_dma *lcd_dma;

void ReviseFramebuffer(uint16_t *framebuffer, int y)
{
    for (int i = 0; i < lcd_dma->GetNumLineInFramebuffer(); ++i)
    {
        for (int j = 0; j < 320; ++j)
        {
            uint16_t r = (uint16_t)(31.0f * (float)(j) / 320.0f);
            uint16_t g = (uint16_t)(63.0f * (float)(j) / 320.0f);
            uint16_t b = (uint16_t)(31.0f * (float)(j) / 320.0f);
            uint16_t c = (r << 11) | (g << 5) | (b << 0);
            c = (c << 8) | (c >> 8);
            framebuffer[320 * i + j] = c;
        }
    }
}

void setup()
{
    lcd_dma = new Lcd_dma(120);
}

void loop()
{
    static uint16_t *framebuffer = lcd_dma->GetFramebuffer();
    for (int y = 0; y < 240; y += lcd_dma->GetNumLineInFramebuffer())
    {
        ReviseFramebuffer(framebuffer, y);
        delay(60);
        lcd_dma->Flip(y);
        framebuffer = lcd_dma->GetFramebuffer();
    }
}