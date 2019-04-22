#include <Lcd_dma.h>

Lcd_dma *lcd_dma;

void setup()
{
    lcd_dma = new Lcd_dma(200, 150);
    lcd_dma->SetBrightness(20);
    lcd_dma->fillScreen(0);
}

void loop()
{
    uint16_t *framebuffer = lcd_dma->GetFramebuffer();
    static int t = 0;
    for (int i = 0; i < lcd_dma->GetHeight(); ++i)
        for (int j = 0; j < lcd_dma->GetWidth(); ++j)
            framebuffer[lcd_dma->GetWidth() * i + j] = ((i + j + t) % 300 < 150) ? 0x00FF8 : 0x1F00;
    ++t;
    delay(10);
    lcd_dma->Flip(60, 45);
}