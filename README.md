# M5Stack_LCD_DMA
M5Stack library (https://github.com/m5stack/M5Stack) has great display class.
It uses polling SPI transactions to transmit data to LCD.
But when you need to calculate efficiently, you want to use interrupt SPI transactions with DMA.
This is simple library which allows you to use it.

## Restrictions
- Drawing area indicated by arguments of constructor (Lcd_dma::Lcd_dma(int width, int height)) is restricted by buffer size you can allocate (see Lcd_dma::CreateFramebuffer()). Generally width x height should be 30000 or less. You can't use PSRAM for the buffer because esp32 doesn't allow you to use external RAM associated with DMA.
- You can't use SD class in M5Stack library freely during using this library. In M5Stack circuit, SPI lines are shared with SD and LCD. Even if you release SPI from Lcd_dma by Lcd_dma::SpiFree(), you can't use SD class freely. I don't know root cause now...