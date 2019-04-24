# M5Stack_LCD_DMA
M5Stack library (https://github.com/m5stack/M5Stack) has great display class.
It uses polling SPI transactions to transmit data to LCD.
But when you need to calculate efficiently, you want to use interrupt SPI transactions with DMA.
This is simple library which allows you to use it.

## Restrictions
- Drawing area indicated by arguments of constructor (Lcd_dma::Lcd_dma(int width, int height)) is restricted by buffer size you can allocate (see Lcd_dma::CreateFramebuffer()). Generally width x height should be 30000 or less. You can't use PSRAM for the buffer because esp32 doesn't allow you to use external RAM associated with DMA.
- If you use SD, you must release SPI from Lcd_dma by calling SpiFree(). After using SD, you must release SPI from SD by calling SPI.end(). Then you can use Lcd_dma by calling SpiRestart(). M5Stack use common lines for SD and LCD. So you can't use them simultaneously.